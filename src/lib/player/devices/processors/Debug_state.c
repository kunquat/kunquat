

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
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
#include <mathnum/conversions.h>
#include <player/devices/processors/Proc_state_utils.h>


static int32_t Debug_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

    // Get pitches
    const Cond_work_buffer* actual_pitches = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Proc_state_get_voice_buffer(proc_state, proc_ts, DEVICE_PORT_TYPE_RECV, 0),
            0);

    // Get output buffers for writing
    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(proc_state, proc_ts, 0, 2, out_buffers);

    Proc_debug* debug = (Proc_debug*)proc->parent.dimpl;
    if (debug->single_pulse)
    {
        if (buf_start < buf_stop)
        {
            const float val = 1.0;
            if (out_buffers[0] != NULL)
                out_buffers[0][buf_start] = val;
            if (out_buffers[1] != NULL)
                out_buffers[1][buf_start] = val;
            Voice_state_set_finished(vstate);

            // We want all single pulses to be included in test buffers,
            // even if another voice replaces us in the channel foreground
            Voice_state_mark_release_data(vstate, buf_start + 1);

            return buf_start + 1;
        }
        return buf_start;
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const double freq = cents_to_Hz(
                Cond_work_buffer_get_value(actual_pitches, i));

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

            Voice_state_mark_release_data(vstate, i + 1);
        }

        if (out_buffers[0] != NULL)
            out_buffers[0][i] = (float)vals[0];
        if (out_buffers[1] != NULL)
            out_buffers[1][i] = (float)vals[1];

        vstate->rel_pos_rem += freq / audio_rate;

        if (!vstate->note_on)
        {
            vstate->noff_pos_rem += freq / audio_rate;
            if (vstate->noff_pos_rem >= 2)
            {
                Voice_state_set_finished(vstate);
                return i + 1;
            }
        }

        if (vstate->rel_pos_rem >= 1)
        {
            ++vstate->pos;
            if (vstate->pos >= 10)
            {
                Voice_state_set_finished(vstate);
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
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Debug_vstate_render_voice;

    return;
}


