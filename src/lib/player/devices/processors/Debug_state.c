

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Debug_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_debug.h>
#include <player/devices/processors/Proc_utils.h>


static int32_t Debug_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

    // Get actual pitches
    const Cond_work_buffer* actual_pitches = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_PITCHES),
            440,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH));

    // Get actual forces
    const Cond_work_buffer* actual_forces = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_FORCES),
            1,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE));

    // Get output buffer for writing
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);
    float* audio_l = Audio_buffer_get_buffer(out_buffer, 0);
    float* audio_r = Audio_buffer_get_buffer(out_buffer, 1);

    Proc_debug* debug = (Proc_debug*)proc->parent.dimpl;
    if (debug->single_pulse)
    {
        if (buf_start < buf_stop)
        {
            const float val = 1.0 * Cond_work_buffer_get_value(actual_forces, buf_start);
            audio_l[buf_start] = val;
            audio_r[buf_start] = val;
            vstate->active = false;
            return buf_start + 1;
        }
        return buf_start;
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_pitch = Cond_work_buffer_get_value(actual_pitches, i);
        const float actual_force = Cond_work_buffer_get_value(actual_forces, i);

        double vals[KQT_BUFFERS_MAX] = { 0 };

        if (vstate->rel_pos == 0)
        {
            vals[0] = vals[1] = 1.0;
            vstate->rel_pos = 1;
        }
        else
        {
            vals[0] = vals[1] = 0.5;
        }

        if (!vstate->note_on)
        {
            vals[0] = -vals[0];
            vals[1] = -vals[1];
        }

        vals[0] *= actual_force;
        vals[1] *= actual_force;

        audio_l[i] = vals[0];
        audio_r[i] = vals[1];

        vstate->rel_pos_rem += actual_pitch / audio_rate;

        if (!vstate->note_on)
        {
            vstate->noff_pos_rem += actual_pitch / audio_rate;
            if (vstate->noff_pos_rem >= 2)
            {
                vstate->active = false;
                return i + 1;
            }
        }

        if (vstate->rel_pos_rem >= 1)
        {
            ++vstate->pos;
            if (vstate->pos >= 10)
            {
                vstate->active = false;
                return i + 1;
            }
            vstate->rel_pos = 0;
            vstate->rel_pos_rem -= floor(vstate->rel_pos_rem);
        }
    }

    return buf_stop;
}


void Debug_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Debug_vstate_render_voice;

    return;
}

