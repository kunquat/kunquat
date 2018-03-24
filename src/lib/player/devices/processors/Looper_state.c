

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
#include <string/common.h>

#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    MODE_RECORD,
    MODE_PLAY,
    MODE_STOP,
} Mode;


typedef struct Looper_pstate
{
    Proc_state parent;

    Mode mode;

    // These values are relative to write_pos; positive values are invalid
    int32_t marker_start;
    int32_t marker_stop;
    float read_pos;

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

    lpstate->mode = MODE_RECORD;
    lpstate->marker_start = 1;
    lpstate->marker_stop = 1;
    lpstate->read_pos = 0;
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
static const int LOOPER_WB_FIXED_SPEED = WORK_BUFFER_IMPL_2;


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

    // Get speed input
    const float* speeds = NULL;
    {
        Work_buffer* speed_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SPEED);
        if (speed_wb == NULL)
        {
            speed_wb = Work_buffers_get_buffer_mut(wbs, LOOPER_WB_FIXED_SPEED);
            float* fixed_speeds = Work_buffer_get_contents_mut(speed_wb);
            for (int32_t i = buf_start; i < buf_stop; ++i)
                fixed_speeds[i] = 1;

            speeds = fixed_speeds;
        }
        else
        {
            speeds = Work_buffer_get_contents(speed_wb);
        }
    }
    rassert(speeds != NULL);

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

    bool enable_playback = false;

    switch (lpstate->mode)
    {
        case MODE_RECORD:
        {
            enable_playback = true;

            // Get total offsets
            float cur_read_pos = lpstate->read_pos;

            for (int32_t i = buf_start, chunk_offset = 0;
                    i < buf_stop;
                    ++i, ++chunk_offset)
            {
                cur_read_pos = clamp(cur_read_pos, (float)-delay_max, 0.0f);
                total_offsets[i] = (float)chunk_offset + cur_read_pos;

                const float speed = speeds[i];
                cur_read_pos += speed - 1;
            }

            lpstate->read_pos = cur_read_pos;

            // Update range markers (for the next update)
            const int32_t frame_count = buf_stop - buf_start;
            if (lpstate->marker_start <= 0)
                lpstate->marker_start -= frame_count;
            if (lpstate->marker_stop <= 0)
                lpstate->marker_stop -= frame_count;
        }
        break;

        case MODE_PLAY:
        {
            // Clamp range markers for safety
            lpstate->marker_start = max(lpstate->marker_start, -delay_max);
            lpstate->marker_stop = max(lpstate->marker_stop, -delay_max);

            enable_playback =
                (lpstate->marker_start < lpstate->marker_stop) &&
                (lpstate->marker_stop <= 0);

            if (enable_playback)
            {
                float cur_read_pos = lpstate->read_pos;

                const float marker_start = (float)lpstate->marker_start;
                const float marker_stop = (float)lpstate->marker_stop;
                const float loop_length = marker_stop - marker_start;

                for (int32_t i = buf_start; i < buf_stop; ++i)
                {
                    if (cur_read_pos >= marker_stop)
                    {
                        const float excess = cur_read_pos - marker_stop;
                        if (isfinite(excess))
                            cur_read_pos = marker_start + fmodf(excess, loop_length);
                        else
                            cur_read_pos = marker_start;

                        rassert(cur_read_pos < marker_stop);
                    }
                    else if (cur_read_pos < marker_start)
                    {
                        const float excess = marker_start - cur_read_pos;
                        if (isfinite(excess))
                        {
                            cur_read_pos = marker_stop - fmodf(excess, loop_length);
                            if (cur_read_pos == marker_stop)
                                cur_read_pos = marker_start;
                        }
                        else
                        {
                            cur_read_pos = marker_start;
                        }

                        rassert(cur_read_pos < marker_stop);
                    }

                    total_offsets[i] = cur_read_pos;

                    const float speed = speeds[i];
                    cur_read_pos += speed;
                }

                lpstate->read_pos = cur_read_pos;
            }
        }
        break;

        case MODE_STOP:
        {
        }
        break;

        default:
            rassert(false);
    }

    if (enable_playback)
    {
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
                    rassert(lpstate->mode == MODE_RECORD);

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
                        if (lpstate->mode == MODE_RECORD)
                        {
                            rassert(next_pos == 0);
                            next_val = in[buf_start];
                        }
                        else
                        {
                            next_val =
                                history[lpstate->write_pos + lpstate->marker_start];
                        }
                    }
                }

                // Create output frame
                out[i] = (float)lerp(cur_val, next_val, remainder);
            }
        }
    }

    if (lpstate->mode == MODE_RECORD)
    {
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
    }

    return;
}


static void Looper_pstate_fire_event(
        Device_state* dstate, const char* event_name, const Value* arg)
{
    rassert(dstate != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    if (string_eq(event_name, "record"))
    {
        lpstate->mode = MODE_RECORD;
        lpstate->read_pos = 0;
    }
    else if (string_eq(event_name, "mark_start"))
    {
        lpstate->marker_start = 0;
        lpstate->marker_stop = 1;
    }
    else if (string_eq(event_name, "mark_stop"))
    {
        lpstate->marker_stop = 0;
    }
    else if (string_eq(event_name, "range_start"))
    {
        // TODO: Set normalised loop start position in the recorded area
        // TODO: How should we handle updating during playback?
    }
    else if (string_eq(event_name, "range_stop"))
    {
        // TODO: Set normalised loop stop position in the recorded area
    }
    else if (string_eq(event_name, "play"))
    {
        lpstate->mode = MODE_PLAY;
        lpstate->read_pos = (float)lpstate->marker_start;
    }
    else if (string_eq(event_name, "stop"))
    {
        lpstate->mode = MODE_STOP;
    }

    return;
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
    lpstate->mode = MODE_RECORD;
    lpstate->marker_start = 1;
    lpstate->marker_stop = 1;
    lpstate->read_pos = 0;
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


