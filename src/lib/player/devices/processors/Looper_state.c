

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Looper_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_looper.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Looper_pstate
{
    Proc_state parent;

    int32_t write_pos;
    Work_buffer* bufs[KQT_BUFFERS_MAX];
} Looper_pstate;


static void del_Looper_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        del_Work_buffer(lpstate->bufs[i]);
        lpstate->bufs[i] = NULL;
    }

    memory_free(lpstate);
}


static void Looper_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = lpstate->bufs[i];
        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
    }

    lpstate->write_pos = 0;

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_SPEED,
    PORT_IN_COUNT,

    PORT_IN_AUDIO_COUNT = PORT_IN_AUDIO_R + 1
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int LOOPER_WB_TOTAL_OFFSETS = WORK_BUFFER_IMPL_1;
//static const int LOOPER_WB_FIXED_SPEED = WORK_BUFFER_IMPL_2;


static void Looper_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    float* in_data[2] = { NULL };
    Proc_state_get_mixed_audio_in_buffers(
            proc_ts, PORT_IN_AUDIO_L, PORT_IN_AUDIO_COUNT, in_data);

    float* out_data[2] = { NULL };
    Proc_state_get_mixed_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_data);

    float* history_data[] =
    {
        Work_buffer_get_contents_mut(lpstate->bufs[0]),
        Work_buffer_get_contents_mut(lpstate->bufs[1]),
    };

    const int32_t history_buf_size = Work_buffer_get_size(lpstate->bufs[0]);
    rassert(history_buf_size == Work_buffer_get_size(lpstate->bufs[1]));
    const int32_t delay_max = history_buf_size - 1;

    float* total_offsets =
        Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_TOTAL_OFFSETS);

    int32_t cur_lpstate_write_pos = lpstate->write_pos;

    const int32_t audio_rate = dstate->audio_rate;

    // Get total offsets
    for (int32_t i = buf_start, chunk_offset = 0; i < buf_stop; ++i, ++chunk_offset)
    {
        const float cur_delay = 0; // TODO: get from speed input
        double delay_frames = cur_delay * (double)audio_rate;
        delay_frames = clamp(delay_frames, 0, delay_max);
        total_offsets[i] = (float)(chunk_offset - delay_frames);
    }

    for (int ch = 0; ch < 2; ++ch)
    {
        float* in = in_data[ch];
        float* out = out_data[ch];
        if ((in == NULL) || (out == NULL))
            continue;

        // Clamp input values to finite range (for interpolation safety)
        for (int32_t i = buf_start; i < buf_stop; ++i)
            in[i] = clamp(in[i], -FLT_MAX, FLT_MAX);

        const float* history = history_data[ch];
        rassert(history != NULL);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float total_offset = total_offsets[i];

            // Get buffer positions
            const int32_t cur_pos = (int32_t)floor(total_offset);
            const double remainder = total_offset - (double)cur_pos;
            rassert(cur_pos <= (int32_t)i);
            rassert(implies(cur_pos == (int32_t)i, remainder == 0));
            const int32_t next_pos = cur_pos + 1;

            // Get audio frames
            double cur_val = 0;
            double next_val = 0;

            if (cur_pos >= 0)
            {
                const int32_t in_cur_pos = buf_start + cur_pos;
                rassert(in_cur_pos < (int32_t)buf_stop);
                cur_val = in[in_cur_pos];

                const int32_t in_next_pos = min(buf_start + next_pos, i);
                rassert(in_next_pos < (int32_t)buf_stop);
                next_val = in[in_next_pos];
            }
            else
            {
                const int32_t cur_history_buf_pos =
                    (cur_lpstate_write_pos + cur_pos + history_buf_size) %
                    history_buf_size;
                rassert(cur_history_buf_pos >= 0);

                cur_val = history[cur_history_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_history_buf_pos =
                        (cur_lpstate_write_pos + next_pos + history_buf_size) %
                        history_buf_size;
                    rassert(next_history_buf_pos >= 0);

                    next_val = history[next_history_buf_pos];
                }
                else
                {
                    rassert(next_pos == 0);
                    next_val = in[buf_start];
                }
            }

            // Create output frame
            out[i] = (float)lerp(cur_val, next_val, remainder);
        }
    }

    // Update the history buffers
    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_data[ch];
        if (in == NULL)
            continue;

        float* history = history_data[ch];
        rassert(history != NULL);

        cur_lpstate_write_pos = lpstate->write_pos;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            history[cur_lpstate_write_pos] = in[i];

            ++cur_lpstate_write_pos;
            if (cur_lpstate_write_pos >= history_buf_size)
            {
                rassert(cur_lpstate_write_pos == history_buf_size);
                cur_lpstate_write_pos = 0;
            }
        }
    }

    lpstate->write_pos = cur_lpstate_write_pos;

    return;
}


static void Looper_pstate_fire_event(
        Device_state* dstate, const char* event_name, const Value* arg)
{
    rassert(dstate != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);

    // TODO: all the things
}


Device_state* new_Looper_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Looper_pstate* lpstate = memory_alloc_item(Looper_pstate);
    if ((lpstate == NULL) ||
            !Proc_state_init(&lpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(lpstate);
        return NULL;
    }

    // Sanitise fields
    lpstate->write_pos = 0;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        lpstate->bufs[i] = NULL;

    lpstate->parent.destroy = del_Looper_pstate;
    lpstate->parent.reset = Looper_pstate_reset;
    lpstate->parent.render_mixed = Looper_pstate_render_mixed;
    lpstate->parent.fire_dev_event = Looper_pstate_fire_event;

    // Initialise
    const Proc_looper* looper = (const Proc_looper*)device->dimpl;
    const int32_t buf_size = (int32_t)(looper->max_rec_time * audio_rate + 1);
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        lpstate->bufs[i] = new_Work_buffer_unbounded(buf_size);
        if (lpstate->bufs[i] == NULL)
        {
            del_Device_state(&lpstate->parent.parent);
            return NULL;
        }
    }

    return (Device_state*)lpstate;
}


bool Looper_pstate_set_max_rec_time(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    ignore(indices);
    ignore(value);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    const Proc_looper* looper = (const Proc_looper*)dstate->device->dimpl;

    const int32_t new_buf_size = (int32_t)(looper->max_rec_time * dstate->audio_rate + 1);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        if (!Work_buffer_resize(lpstate->bufs[i], new_buf_size))
            return false;
    }

    Looper_pstate_reset(dstate);

    return true;
}


