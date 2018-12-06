

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
#include <init/devices/Device_impl.h>
#include <init/devices/Device_port_groups.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_debug.h>
#include <mathnum/conversions.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>


void Debug_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);
    rassert(groups != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


int32_t Debug_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

    // Get pitches
    const Cond_work_buffer* actual_pitches = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, 0, NULL),
            0);

    // Get output buffers for writing
    float* out_buffer = NULL;
    {
        Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, 0, NULL);
        if (out_wb != NULL)
        {
            rassert(Work_buffer_get_sub_count(out_wb) == 2);
            rassert(Work_buffer_get_stride(out_wb) == 2);

            out_buffer = Work_buffer_get_contents_mut(out_wb, 0);
            Work_buffer_mark_valid(out_wb, 0);
            Work_buffer_mark_valid(out_wb, 1);
        }
    }

    Proc_debug* debug = (Proc_debug*)proc->parent.dimpl;
    if (debug->single_pulse)
    {
        if (vstate->pos == 1)
        {
            vstate->active = false;
            return 0;
        }

        const float val = 1.0;
        if (out_buffer != NULL)
        {
            out_buffer[0] = val;
            out_buffer[1] = val;
        }

        // We want all single pulses to be included in test buffers,
        // even if another voice replaces us in the channel foreground
        Voice_state_set_keep_alive_stop(vstate, 1);

        vstate->pos = 1;

        return 1;
    }

    if ((vstate->pos >= 10) || (!vstate->note_on && vstate->noff_pos_rem >= 2))
    {
        vstate->active = false;
        return 0;
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;

    for (int32_t i = 0; i < frame_count; ++i)
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

        if (out_buffer != NULL)
        {
            out_buffer[i * 2] = (float)vals[0];
            out_buffer[i * 2 + 1] = (float)vals[1];
        }

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

    Voice_state_set_keep_alive_stop(vstate, frame_count);

    return frame_count;
}


void Debug_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    return;
}


