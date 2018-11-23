

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
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
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_debug.h>
#include <mathnum/conversions.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>


int32_t Debug_vstate_render_voice(
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
            Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, 0, NULL),
            0);

    // Get output buffers for writing
    float* out_buffers[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, ch, NULL);
        if (out_wb != NULL)
            out_buffers[ch] = Work_buffer_get_contents_mut(out_wb, 0);
    }

    Proc_debug* debug = (Proc_debug*)proc->parent.dimpl;
    if (debug->single_pulse)
    {
        if (buf_start < buf_stop)
        {
            if (vstate->pos == 1)
            {
                vstate->active = false;
                return buf_start;
            }

            const float val = 1.0;
            if (out_buffers[0] != NULL)
                out_buffers[0][buf_start] = val;
            if (out_buffers[1] != NULL)
                out_buffers[1][buf_start] = val;

            // We want all single pulses to be included in test buffers,
            // even if another voice replaces us in the channel foreground
            Voice_state_set_keep_alive_stop(vstate, buf_start + 1);

            vstate->pos = 1;

            return buf_start + 1;
        }
        return buf_start;
    }

    if ((vstate->pos >= 10) || (!vstate->note_on && vstate->noff_pos_rem >= 2))
    {
        vstate->active = false;
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
                Voice_state_set_keep_alive_stop(vstate, i + 1);
                return i + 1;
            }
        }

        if (vstate->rel_pos_rem >= 1)
        {
            ++vstate->pos;
            if (vstate->pos >= 10)
            {
                Voice_state_set_keep_alive_stop(vstate, i + 1);
                return i + 1;
            }
            vstate->rel_pos = 0;
            vstate->rel_pos_rem -= floor(vstate->rel_pos_rem);
        }
    }

    Voice_state_set_keep_alive_stop(vstate, buf_stop);

    return buf_stop;
}


void Debug_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    return;
}


