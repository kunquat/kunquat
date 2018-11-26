

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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define RANGES_MAX 16


typedef enum
{
    MODE_RECORD,
    MODE_PLAY,
    MODE_MIX,
    MODE_STOP,
    MODE_BYPASS,
} Mode;


typedef struct Range
{
    // These values are relative to head_pos; positive values are invalid
    int32_t start;
    int32_t stop;
} Range;


static void Range_init(Range* range)
{
    rassert(range != NULL);

    range->start = 1;
    range->stop = 1;

    return;
}


static void Range_update(Range* range, int32_t frame_count)
{
    rassert(range != NULL);
    rassert(frame_count >= 0);

    if (range->start <= 0)
        range->start -= frame_count;
    if (range->stop <= 0)
        range->stop -= frame_count;

    return;
}


static bool Range_is_valid(const Range* range)
{
    rassert(range != NULL);
    return (range->start < range->stop && range->stop <= 0);
}


typedef struct Mode_context
{
    Mode mode;
    bool is_fading;

    // These values are relative to head_pos; positive values are invalid
    float read_pos;
    int32_t write_pos;
    Range range;

    const Proc_looper* looper;
} Mode_context;


static void Mode_context_init(Mode_context* context, const Proc_looper* looper)
{
    rassert(context != NULL);
    rassert(looper != NULL);

    context->mode = MODE_STOP;
    context->is_fading = false;
    context->read_pos = 0;
    context->write_pos = 0;
    Range_init(&context->range);

    context->looper = looper;

    return;
}


static void Mode_context_update(
        Mode_context* context,
        float* total_offsets,
        const float* speeds,
        bool is_head_pos_moving,
        int32_t delay_max,
        int32_t frame_count)
{
    rassert(context != NULL);
    rassert(total_offsets != NULL);
    rassert(speeds != NULL);
    rassert(delay_max > 0);
    rassert(frame_count > 0);

    switch (context->mode)
    {
        case MODE_RECORD:
        {
            rassert(is_head_pos_moving);

            // Get total offsets
            float cur_read_pos = context->read_pos;

            for (int32_t i = 0, chunk_offset = 0; i < frame_count; ++i, ++chunk_offset)
            {
                cur_read_pos = clamp(cur_read_pos, (float)-delay_max, 0.0f);
                total_offsets[i] = (float)chunk_offset + cur_read_pos;

                const float speed = speeds[i];
                cur_read_pos += speed - 1;
            }

            context->read_pos = cur_read_pos;

            context->write_pos = 0;
        }
        break;

        case MODE_PLAY:
        {
            const float marker_start = (float)clamp(context->range.start, -delay_max, 0);
            const float marker_stop = (float)clamp(context->range.stop, -delay_max, 0);

            const float loop_length = marker_stop - marker_start;
            if (loop_length <= 0)
            {
                context->mode = MODE_STOP;

                if ((context->read_pos <= 0) && is_head_pos_moving)
                {
                    context->read_pos -= (float)frame_count;
                    context->read_pos = max(context->read_pos, (float)-delay_max);
                }

                return;
            }

            if ((marker_start < marker_stop) && (marker_stop <= 0))
            {
                float cur_read_pos = context->read_pos;

                for (int32_t i = 0; i < frame_count; ++i)
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

                if (is_head_pos_moving)
                    cur_read_pos -= (float)frame_count;

                context->read_pos = cur_read_pos;
            }
        }
        break;

        case MODE_MIX:
        {
            const float marker_start = (float)clamp(context->range.start, -delay_max, 0);
            const float marker_stop = (float)clamp(context->range.stop, -delay_max, 0);

            const float loop_length = marker_stop - marker_start;
            if (loop_length <= 0)
            {
                context->mode = MODE_STOP;

                if ((context->read_pos <= 0) && is_head_pos_moving)
                {
                    context->read_pos -= (float)frame_count;
                    context->read_pos = max(context->read_pos, (float)-delay_max);
                }

                context->write_pos = (int32_t)context->read_pos;

                return;
            }

            if (context->write_pos >= context->range.stop)
            {
                const int32_t excess = context->write_pos - context->range.stop + 1;
                context->write_pos =
                    context->range.start +
                    (excess % (context->range.stop - context->range.start));
            }

            if ((marker_start < marker_stop) && (marker_stop <= 0))
            {
                int32_t cur_write_pos = context->write_pos;
                float cur_read_pos = context->read_pos;

                for (int32_t i = 0; i < frame_count; ++i)
                {
                    const int32_t next_write_pos_unwrapped = cur_write_pos + 1;

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

                    // Prevent moving past write_pos
                    {
                        float next_read_pos = cur_read_pos + speed;
                        float limit = (float)next_write_pos_unwrapped;

                        if (speed > 1)
                        {
                            while (limit < cur_read_pos)
                                limit += loop_length;

                            next_read_pos = min(next_read_pos, limit);
                        }
                        else if (speed < 1)
                        {
                            while (limit >= cur_read_pos)
                                limit -= loop_length;
                            limit += 1;

                            next_read_pos = max(next_read_pos, limit);
                        }

                        cur_read_pos = next_read_pos;
                    }

                    if (next_write_pos_unwrapped >= context->range.stop)
                        cur_write_pos = context->range.start;
                    else
                        cur_write_pos = next_write_pos_unwrapped;
                }

                if (is_head_pos_moving)
                    cur_read_pos -= (float)frame_count;

                context->read_pos = cur_read_pos;

                // Note: context->write_pos is updated in Mode_context_render
            }
        }
        break;

        case MODE_STOP:
        case MODE_BYPASS:
        {
            if (is_head_pos_moving)
            {
                if (context->read_pos <= 0)
                {
                    context->read_pos -= (float)frame_count;
                    context->read_pos = max(context->read_pos, (float)-delay_max);
                }

                if (context->write_pos <= 0)
                {
                    context->write_pos -= frame_count;
                    context->write_pos = max(context->write_pos, -delay_max);
                }
            }
        }
        break;

        default:
            rassert(false);
    }

    return;
}


static bool Mode_context_is_playback_enabled(const Mode_context* context)
{
    rassert(context != NULL);

    return
        (context->mode == MODE_RECORD) ||
        (context->mode == MODE_BYPASS) ||
        ((context->mode == MODE_PLAY || context->mode == MODE_MIX) &&
         Range_is_valid(&context->range));
}


static void Mode_context_render(
        Mode_context* context,
        float* out_bufs[2],
        float* in_bufs[2],
        float* history_bufs[2],
        const float* total_offsets,
        bool is_head_pos_moving,
        int32_t history_buf_size,
        int32_t head_pos,
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(context != NULL);
    rassert(context->looper != NULL);
    rassert(out_bufs != NULL);
    rassert(in_bufs != NULL);
    rassert(history_bufs != NULL);
    rassert(total_offsets != NULL);
    rassert(frame_count > 0);

    if (!Mode_context_is_playback_enabled(context))
        return;

    if (context->mode == MODE_BYPASS)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* in = in_bufs[ch];
            float* out = out_bufs[ch];
            if ((in == NULL) || (out == NULL))
                continue;

            for (int32_t i = 0; i < frame_count; ++i)
                out[i] = in[i];
        }

        return;
    }

    if (context->mode == MODE_MIX)
    {
        // Mix input into playback region
        // TODO: handle crossfade
        int32_t write_pos = context->write_pos;

        for (int ch = 0; ch < 2; ++ch)
        {
            const float* in = in_bufs[ch];
            if (in == NULL)
                continue;

            float* history = history_bufs[ch];
            rassert(history != NULL);

            write_pos = context->write_pos;

            for (int32_t i = 0; i < frame_count; ++i)
            {
                const int32_t history_buf_pos =
                    (head_pos + write_pos + history_buf_size) % history_buf_size;
                history[history_buf_pos] += in[i];

                ++write_pos;
                if (write_pos >= context->range.stop)
                    write_pos = context->range.start;
            }
        }

        context->write_pos = write_pos;
        if (is_head_pos_moving)
            context->write_pos -= frame_count;
    }

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_bufs[ch];
        float* out = out_bufs[ch];
        if ((in == NULL) || (out == NULL))
            continue;

        const float* history = history_bufs[ch];
        rassert(history != NULL);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float total_offset = total_offsets[i];

            // Get buffer positions
            const int32_t cur_pos = (int32_t)floor(total_offset);
            const double remainder = total_offset - (double)cur_pos;
            rassert(cur_pos <= (int32_t)i);
            rassert(implies(cur_pos == (int32_t)i, remainder == 0));
            const int32_t next_pos = cur_pos + 1; // TODO: fix for playback

            // Get audio frames
            double cur_val = 0;
            double next_val = 0;

            if (cur_pos >= 0)
            {
                const int32_t in_cur_pos = cur_pos;
                rassert(in_cur_pos < frame_count);
                cur_val = in[in_cur_pos];

                const int32_t in_next_pos = min(next_pos, i);
                rassert(in_next_pos < frame_count);
                next_val = in[in_next_pos];
            }
            else
            {
                const int32_t cur_history_buf_pos =
                    (head_pos + cur_pos + history_buf_size) % history_buf_size;
                rassert(cur_history_buf_pos >= 0);

                cur_val = history[cur_history_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_history_buf_pos =
                        (head_pos + next_pos + history_buf_size) % history_buf_size;
                    rassert(next_history_buf_pos >= 0);

                    next_val = history[next_history_buf_pos];
                }
                else
                {
                    if (context->mode == MODE_RECORD)
                    {
                        rassert(next_pos == 0);
                        next_val = in[0];
                    }
                    else
                    {
                        next_val = history[head_pos + context->range.start];
                    }
                }
            }

            // Create output frame
            out[i] = (float)lerp(cur_val, next_val, remainder);
        }
    }

    const float xfade_time = (float)context->looper->play_xfade_time;
    if ((context->mode == MODE_PLAY) && (xfade_time > 0))
    {
        // Process playback crossfade
        const float marker_start = (float)context->range.start;
        const float marker_stop = (float)context->range.stop;
        const float loop_length = marker_stop - marker_start;

        float xfade_length = xfade_time * (float)audio_rate;
        xfade_length = min(xfade_length, loop_length);

        for (int ch = 0; ch < 2; ++ch)
        {
            const float* in = in_bufs[ch];
            float* out = out_bufs[ch];
            if ((in == NULL) || (out == NULL))
                continue;

            const float* history = history_bufs[ch];
            rassert(history != NULL);

            for (int32_t i = 0; i < frame_count; ++i)
            {
                const float total_offset = total_offsets[i];
                const float dist_to_end = marker_stop - total_offset;
                const float xfade_progress =
                    (xfade_length - dist_to_end) / xfade_length;
                if (xfade_progress <= 0)
                    continue;

                rassert(xfade_progress <= 1);

                // Get fade-in buffer positions
                const float fadein_total_offset = total_offset - loop_length;
                rassert(fadein_total_offset < 0);
                const int32_t fadein_cur_pos = (int32_t)floor(fadein_total_offset);
                const float remainder = fadein_total_offset - (float)fadein_cur_pos;
                rassert(fadein_cur_pos <= (int32_t)i);
                rassert(implies(fadein_cur_pos == (int32_t)i, remainder == 0));
                const int32_t fadein_next_pos = fadein_cur_pos + 1;
                rassert(fadein_next_pos < 0);

                // Get audio frames
                const int32_t cur_history_buf_pos =
                    (head_pos + fadein_cur_pos + history_buf_size) % history_buf_size;
                rassert(cur_history_buf_pos >= 0);

                const float fadein_cur_val = history[cur_history_buf_pos];

                const int32_t next_history_buf_pos =
                    (head_pos + fadein_next_pos + history_buf_size) %
                    history_buf_size;
                rassert(next_history_buf_pos >= 0);

                const float fadein_next_val = history[next_history_buf_pos];

                // Mix crossfading frame with existing output
                const float fadein_val =
                    lerp(fadein_cur_val, fadein_next_val, remainder);
                out[i] = lerp(out[i], fadein_val, xfade_progress);
            }
        }
    }

    return;
}


typedef struct Looper_pstate
{
    Proc_state parent;

    int main_context;
    Mode_context contexts[2];

    float xfade_progress;
    bool is_prev_speed_const;
    float prev_speed;

    int last_range_index;
    Range ranges[RANGES_MAX];

    int32_t head_pos;
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
    const Proc_looper* looper = (const Proc_looper*)dstate->device->dimpl;

    lpstate->main_context = 0;
    for (int i = 0; i < 2; ++i)
        Mode_context_init(&lpstate->contexts[i], looper);
    lpstate->contexts[lpstate->main_context].mode = MODE_RECORD;

    lpstate->xfade_progress = 1;
    lpstate->is_prev_speed_const = true;
    lpstate->prev_speed = 0;

    lpstate->last_range_index = 0;
    for (int i = 0; i < RANGES_MAX; ++i)
        Range_init(&lpstate->ranges[i]);

    lpstate->head_pos = 0;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = lpstate->bufs[i];
        Work_buffer_clear(buf, 0, 0, Work_buffer_get_size(buf));
    }

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


static const int LOOPER_WB_TOTAL_OFFSETS    = WORK_BUFFER_IMPL_1;
static const int LOOPER_WB_FADING_OUT_L     = WORK_BUFFER_IMPL_2;
static const int LOOPER_WB_FADING_OUT_R     = WORK_BUFFER_IMPL_3;
static const int LOOPER_WB_TEMP_SPEED       = WORK_BUFFER_IMPL_4;


static Mode_context* get_main_context(Looper_pstate* lpstate)
{
    rassert(lpstate != NULL);
    return &lpstate->contexts[lpstate->main_context];
}


static Mode_context* get_fading_context(Looper_pstate* lpstate)
{
    rassert(lpstate != NULL);
    return &lpstate->contexts[1 - lpstate->main_context];
}


static void Looper_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Looper_pstate* lpstate = (Looper_pstate*)dstate;

    float* in_data[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* in_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch, NULL);
        if ((in_wb != NULL) && Work_buffer_is_valid(in_wb, 0))
            in_data[ch] = Work_buffer_get_contents_mut(in_wb, 0);
    }

    float* out_data[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch, NULL);
        if (out_wb != NULL)
        {
            Work_buffer_clear(out_wb, 0, 0, frame_count); // TODO: perhaps optimisable?
            out_data[ch] = Work_buffer_get_contents_mut(out_wb, 0);
        }
    }

    // Get speed input
    const float* speeds = NULL;
    Work_buffer* speeds_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SPEED, NULL);
    if ((speeds_wb == NULL) || !Work_buffer_is_valid(speeds_wb, 0))
    {
        speeds_wb = Work_buffers_get_buffer_mut(wbs, LOOPER_WB_TEMP_SPEED, 1);
        float* fixed_speeds = Work_buffer_get_contents_mut(speeds_wb, 0);
        for (int32_t i = 0; i < frame_count; ++i)
            fixed_speeds[i] = 1;
        Work_buffer_set_const_start(speeds_wb, 0, 0);

        speeds = fixed_speeds;
    }
    else
    {
        speeds = Work_buffer_get_contents(speeds_wb, 0);
    }
    rassert(speeds != NULL);

    float* history_data[] =
    {
        Work_buffer_get_contents_mut(lpstate->bufs[0], 0),
        Work_buffer_get_contents_mut(lpstate->bufs[1], 0),
    };

    const int32_t history_buf_size = Work_buffer_get_size(lpstate->bufs[0]);
    rassert(history_buf_size == Work_buffer_get_size(lpstate->bufs[1]));
    const int32_t delay_max = history_buf_size - 1;

    float* total_offsets =
        Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_TOTAL_OFFSETS);

    int32_t cur_lpstate_head_pos = lpstate->head_pos;

    Mode_context* main_context = get_main_context(lpstate);
    Mode_context* fading_context = get_fading_context(lpstate);

    const bool is_head_pos_moving =
        (main_context->mode == MODE_RECORD) || (fading_context->mode == MODE_RECORD);

    bool enable_playback = false;

    if (lpstate->xfade_progress >= 1)
    {
        enable_playback = Mode_context_is_playback_enabled(main_context);
    }
    else
    {
        enable_playback =
            Mode_context_is_playback_enabled(main_context) ||
            Mode_context_is_playback_enabled(fading_context);
    }

    if (enable_playback)
    {
        // Clamp input values to finite range (for interpolation safety)
        for (int ch = 0; ch < 2; ++ch)
        {
            float* in = in_data[ch];
            if (in != NULL)
            {
                for (int32_t i = 0; i < frame_count; ++i)
                    in[i] = clamp(in[i], -FLT_MAX, FLT_MAX);
            }
        }
    }

    // Main state context updates
    Mode_context_update(
            main_context,
            total_offsets,
            speeds,
            is_head_pos_moving,
            delay_max,
            frame_count);

    Mode_context_render(
            main_context,
            out_data,
            in_data,
            history_data,
            total_offsets,
            is_head_pos_moving,
            history_buf_size,
            cur_lpstate_head_pos,
            frame_count,
            dstate->audio_rate);

    // Store previous speed value to improve crossfading behaviour
    const float prev_speed = lpstate->prev_speed;
    const bool is_prev_speed_const = lpstate->is_prev_speed_const;
    lpstate->prev_speed = speeds[frame_count - 1];
    lpstate->is_prev_speed_const =
        (Work_buffer_get_const_start(speeds_wb, 0) < frame_count);

    if (lpstate->xfade_progress < 1)
    {
        // Process crossfade
        const Proc_looper* looper = (Proc_looper*)dstate->device->dimpl;
        const float xfade_time = (float)looper->state_xfade_time;

        int32_t xfade_buf_stop = 0;
        float xfade_step = 0;
        if (xfade_time > 0)
        {
            xfade_step = 1.0f / (xfade_time * (float)dstate->audio_rate);
            const float xfade_left = 1 - lpstate->xfade_progress;
            xfade_buf_stop = (int32_t)ceilf(xfade_left / xfade_step);
            xfade_buf_stop = min(xfade_buf_stop, frame_count);
        }

        if (xfade_buf_stop > 0)
        {
            float* xfade_out_data[2] =
            {
                Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_FADING_OUT_L),
                Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_FADING_OUT_R),
            };

            for (int ch = 0; ch < 2; ++ch)
            {
                float* xfade_out = xfade_out_data[ch];
                for (int i = 0; i < xfade_buf_stop; ++i)
                    xfade_out[i] = 0;
            }

            Work_buffer* xfade_speeds_wb =
                Work_buffers_get_buffer_mut(wbs, LOOPER_WB_TEMP_SPEED, 1);
            const float* xfade_speeds = NULL;
            if (is_prev_speed_const)
            {
                // Use previous speed value and assume that further changes in
                // speed are only meant for the main context
                float* new_xfade_speeds =
                    Work_buffer_get_contents_mut(xfade_speeds_wb, 0);
                for (int32_t i = 0; i < xfade_buf_stop; ++i)
                    new_xfade_speeds[i] = prev_speed;

                xfade_speeds = new_xfade_speeds;
            }
            else
            {
                // Limit the absolute speed to whatever previous speed was
                float* used_speeds = Work_buffer_get_contents_mut(speeds_wb, 0);
                const float max_abs_speed = fabsf(prev_speed);
                for (int32_t i = 0; i < xfade_buf_stop; ++i)
                    used_speeds[i] =
                        clamp(used_speeds[i], -max_abs_speed, max_abs_speed);

                xfade_speeds = used_speeds;
            }
            rassert(xfade_speeds != NULL);

            Mode_context_update(
                    fading_context,
                    total_offsets,
                    xfade_speeds,
                    is_head_pos_moving,
                    delay_max,
                    xfade_buf_stop);

            Mode_context_render(
                    fading_context,
                    xfade_out_data,
                    in_data,
                    history_data,
                    total_offsets,
                    is_head_pos_moving,
                    history_buf_size,
                    cur_lpstate_head_pos,
                    xfade_buf_stop,
                    dstate->audio_rate);

            float xfade_progress = lpstate->xfade_progress;

            for (int ch = 0; ch < 2; ++ch)
            {
                xfade_progress = lpstate->xfade_progress;

                float* out = out_data[ch];
                if (out == NULL)
                    continue;

                const float* xfade_out = xfade_out_data[ch];

                for (int32_t i = 0; i < xfade_buf_stop; ++i)
                {
                    out[i] = lerp(xfade_out[i], out[i], xfade_progress);

                    xfade_progress += xfade_step;
                    xfade_progress = min(xfade_progress, 1.0f);
                }
            }

            lpstate->xfade_progress = xfade_progress;

            if (lpstate->xfade_progress >= 1)
            {
                fading_context->mode = MODE_STOP;
            }
        }
        else
        {
            lpstate->xfade_progress = 1;
            fading_context->mode = MODE_STOP;
        }
    }

    if (is_head_pos_moving)
    {
        // Update the history buffers
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* in = in_data[ch];
            if (in == NULL)
                continue;

            float* history = history_data[ch];
            rassert(history != NULL);

            cur_lpstate_head_pos = lpstate->head_pos;

            for (int32_t i = 0; i < frame_count; ++i)
            {
                history[cur_lpstate_head_pos] = in[i];

                ++cur_lpstate_head_pos;
                if (cur_lpstate_head_pos >= history_buf_size)
                {
                    rassert(cur_lpstate_head_pos == history_buf_size);
                    cur_lpstate_head_pos = 0;
                }
            }
        }

        lpstate->head_pos = cur_lpstate_head_pos;

        // Update range markers for the next update
        {
            for (int i = 0; i < RANGES_MAX; ++i)
                Range_update(&lpstate->ranges[i], frame_count);

            Range_update(&main_context->range, frame_count);
            Range_update(&fading_context->range, frame_count);
        }
    }

    return;
}


static void switch_context(Looper_pstate* lpstate)
{
    rassert(lpstate != NULL);

    lpstate->main_context = 1 - lpstate->main_context;

    Mode_context* main_context = get_main_context(lpstate);
    Mode_context* fading_context = get_fading_context(lpstate);

    Mode_context_init(main_context, main_context->looper);
    main_context->mode = fading_context->mode;
    main_context->read_pos = fading_context->read_pos;
    main_context->write_pos = fading_context->write_pos;
    main_context->range = fading_context->range;

    fading_context->is_fading = true;

    lpstate->xfade_progress = 0;

    return;
}


static void update_last_range_index(Looper_pstate* lpstate, const Value* arg)
{
    rassert(lpstate != NULL);
    rassert(arg != NULL);

    if (arg->type != VALUE_TYPE_INT)
        return;

    if (arg->value.int_type < 0 || arg->value.int_type >= RANGES_MAX)
        return;

    lpstate->last_range_index = (int)arg->value.int_type;

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
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        main_context->mode = MODE_RECORD;
        main_context->read_pos = 0;
        main_context->write_pos = 0;
    }
    else if (string_eq(event_name, "mark_start"))
    {
        update_last_range_index(lpstate, arg);

        Range* range = &lpstate->ranges[lpstate->last_range_index];
        range->start = 0;
        range->stop = 1;
    }
    else if (string_eq(event_name, "mark_stop"))
    {
        update_last_range_index(lpstate, arg);

        Range* range = &lpstate->ranges[lpstate->last_range_index];
        range->stop = 0;
    }
    else if (string_eq(event_name, "play"))
    {
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        update_last_range_index(lpstate, arg);

        main_context->mode = MODE_PLAY;
        main_context->range = lpstate->ranges[lpstate->last_range_index];
        if (main_context->read_pos >= (float)main_context->range.stop)
            main_context->read_pos = (float)main_context->range.start;
    }
    else if (string_eq(event_name, "mix"))
    {
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        update_last_range_index(lpstate, arg);

        main_context->mode = MODE_MIX;
        main_context->range = lpstate->ranges[lpstate->last_range_index];
        if (main_context->read_pos >= (float)main_context->range.stop)
            main_context->read_pos = (float)main_context->range.start;
        main_context->write_pos = (int32_t)ceilf(main_context->read_pos);
    }
    else if (string_eq(event_name, "stop"))
    {
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        main_context->mode = MODE_STOP;
    }
    else if (string_eq(event_name, "bypass"))
    {
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        main_context->mode = MODE_BYPASS;
    }

    return;
}


Device_state* new_Looper_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    const Proc_looper* looper = (const Proc_looper*)device->dimpl;

    Looper_pstate* lpstate = memory_alloc_item(Looper_pstate);
    if ((lpstate == NULL) ||
            !Proc_state_init(&lpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(lpstate);
        return NULL;
    }

    // Sanitise fields
    lpstate->main_context = 0;
    for (int i = 0; i < 2; ++i)
        Mode_context_init(&lpstate->contexts[i], looper);
    lpstate->contexts[lpstate->main_context].mode = MODE_RECORD;

    lpstate->xfade_progress = 1;
    lpstate->is_prev_speed_const = true;
    lpstate->prev_speed = 0;

    lpstate->last_range_index = 0;
    for (int i = 0; i < RANGES_MAX; ++i)
        Range_init(&lpstate->ranges[i]);

    lpstate->head_pos = 0;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        lpstate->bufs[i] = NULL;

    lpstate->parent.destroy = del_Looper_pstate;
    lpstate->parent.reset = Looper_pstate_reset;
    lpstate->parent.render_mixed = Looper_pstate_render_mixed;
    lpstate->parent.fire_dev_event = Looper_pstate_fire_event;

    // Initialise
    const int32_t buf_size = (int32_t)(looper->max_rec_time * audio_rate + 1);
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        lpstate->bufs[i] = new_Work_buffer(buf_size, 1);
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


