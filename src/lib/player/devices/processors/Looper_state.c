

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
#include <stdlib.h>


typedef enum
{
    MODE_RECORD,
    MODE_PLAY,
    MODE_STOP,
} Mode;


typedef struct Mode_context
{
    Mode mode;

    // This value is relative to write_pos; positive values are invalid
    float read_pos;
} Mode_context;


static void Mode_context_init(Mode_context* context)
{
    rassert(context != NULL);

    context->mode = MODE_STOP;
    context->read_pos = 0;

    return;
}


static void Mode_context_update(
        Mode_context* context,
        float* total_offsets,
        const float* speeds,
        bool is_recording,
        int32_t delay_max,
        int32_t init_marker_start,
        int32_t init_marker_stop,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(context != NULL);
    rassert(total_offsets != NULL);
    rassert(speeds != NULL);
    rassert(delay_max > 0);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    switch (context->mode)
    {
        case MODE_RECORD:
        {
            rassert(is_recording);

            // Get total offsets
            float cur_read_pos = context->read_pos;

            for (int32_t i = buf_start, chunk_offset = 0;
                    i < buf_stop;
                    ++i, ++chunk_offset)
            {
                cur_read_pos = clamp(cur_read_pos, (float)-delay_max, 0.0f);
                total_offsets[i] = (float)chunk_offset + cur_read_pos;

                const float speed = speeds[i];
                cur_read_pos += speed - 1;
            }

            context->read_pos = cur_read_pos;
        }
        break;

        case MODE_PLAY:
        {
            const float marker_start = (float)clamp(init_marker_start, -delay_max, 0);
            const float marker_stop = (float)clamp(init_marker_stop, -delay_max, 0);

            const float loop_length = marker_stop - marker_start;
            if (loop_length <= 0)
            {
                context->mode = MODE_STOP;

                if ((context->read_pos <= 0) && is_recording)
                {
                    const int32_t frame_count = (buf_stop - buf_start);
                    context->read_pos -= (float)frame_count;
                    context->read_pos = max(context->read_pos, (float)-delay_max);
                }

                return;
            }

            if ((marker_start < marker_stop) && (marker_stop <= 0))
            {
                float cur_read_pos = context->read_pos;

                static int frames_since_last_jump = 0;

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
                        ++frames_since_last_jump;
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
                    else
                        ++frames_since_last_jump;

                    total_offsets[i] = cur_read_pos;

                    const float speed = speeds[i];
                    cur_read_pos += speed;
                }

                if (is_recording)
                    cur_read_pos -= (float)(buf_stop - buf_start);

                context->read_pos = cur_read_pos;
            }
        }
        break;

        case MODE_STOP:
        {
            if ((context->read_pos <= 0) && is_recording)
            {
                const int32_t frame_count = (buf_stop - buf_start);
                context->read_pos -= (float)frame_count;
                context->read_pos = max(context->read_pos, (float)-delay_max);
            }
        }
        break;

        default:
            rassert(false);
    }

    return;
}


static bool Mode_context_is_playback_enabled(
        const Mode_context* context,
        int32_t init_marker_start,
        int32_t init_marker_stop)
{
    rassert(context != NULL);

    return
        (context->mode == MODE_RECORD) ||
        (context->mode == MODE_PLAY &&
         (init_marker_start < init_marker_stop && init_marker_stop <= 0));
}


static void Mode_context_render(
        Mode_context* context,
        float* out_bufs[2],
        float* in_bufs[2],
        float* history_bufs[2],
        const float* total_offsets,
        int32_t history_buf_size,
        int32_t write_pos,
        int32_t init_marker_start,
        int32_t init_marker_stop,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(context != NULL);
    rassert(out_bufs != NULL);
    rassert(in_bufs != NULL);
    rassert(history_bufs != NULL);
    rassert(total_offsets != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    const bool enable_playback =
        (context->mode == MODE_RECORD) ||
        (context->mode == MODE_PLAY &&
         (init_marker_start < init_marker_stop && init_marker_stop <= 0));

    if (!enable_playback)
        return;

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_bufs[ch];
        float* out = out_bufs[ch];
        if ((in == NULL) || (out == NULL))
            continue;

        const float* history = history_bufs[ch];
        rassert(history != NULL);

        for (int32_t i = buf_start; i < buf_stop; ++i)
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
                    (write_pos + cur_pos + history_buf_size) % history_buf_size;
                rassert(cur_history_buf_pos >= 0);

                cur_val = history[cur_history_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_history_buf_pos =
                        (write_pos + next_pos + history_buf_size) % history_buf_size;
                    rassert(next_history_buf_pos >= 0);

                    next_val = history[next_history_buf_pos];
                }
                else
                {
                    if (context->mode == MODE_RECORD)
                    {
                        rassert(next_pos == 0);
                        next_val = in[buf_start];
                    }
                    else
                    {
                        next_val = history[write_pos + init_marker_start];
                    }
                }
            }

            // Create output frame
            out[i] = (float)lerp(cur_val, next_val, remainder);
        }
    }

    return;
}


typedef struct Looper_pstate
{
    Proc_state parent;

    int main_context;
    float xfade_progress;
    Mode_context contexts[2];

    // These values are relative to write_pos; positive values are invalid
    int32_t marker_start;
    int32_t marker_stop;

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

    lpstate->main_context = 0;
    lpstate->xfade_progress = 1;
    for (int i = 0; i < 2; ++i)
        Mode_context_init(&lpstate->contexts[i]);
    lpstate->contexts[lpstate->main_context].mode = MODE_RECORD;

    lpstate->marker_start = 1;
    lpstate->marker_stop = 1;
    lpstate->write_pos = 0;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = lpstate->bufs[i];
        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
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
static const int LOOPER_WB_FIXED_SPEED      = WORK_BUFFER_IMPL_4;


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

    Mode_context* main_context = get_main_context(lpstate);
    Mode_context* fading_context = get_fading_context(lpstate);

    const bool is_recording =
        (main_context->mode == MODE_RECORD) || (fading_context->mode == MODE_RECORD);

    bool enable_playback = false;

    if (lpstate->xfade_progress >= 1)
    {
        enable_playback = Mode_context_is_playback_enabled(
                main_context, lpstate->marker_start, lpstate->marker_stop);
    }
    else
    {
        enable_playback =
            Mode_context_is_playback_enabled(
                    main_context, lpstate->marker_start, lpstate->marker_stop) ||
            Mode_context_is_playback_enabled(
                    fading_context, lpstate->marker_start, lpstate->marker_stop);
    }

    if (enable_playback)
    {
        // Clamp input values to finite range (for interpolation safety)
        for (int ch = 0; ch < 2; ++ch)
        {
            float* in = in_data[ch];
            if (in != NULL)
            {
                for (int32_t i = buf_start; i < buf_stop; ++i)
                    in[i] = clamp(in[i], -FLT_MAX, FLT_MAX);
            }
        }
    }

    Mode_context_update(
            main_context,
            total_offsets,
            speeds,
            is_recording,
            delay_max,
            lpstate->marker_start,
            lpstate->marker_stop,
            buf_start,
            buf_stop);

    if (enable_playback)
    {
        Mode_context_render(
                main_context,
                out_data,
                in_data,
                history_data,
                total_offsets,
                history_buf_size,
                cur_lpstate_write_pos,
                lpstate->marker_start,
                lpstate->marker_stop,
                buf_start,
                buf_stop);
    }

    if (lpstate->xfade_progress < 1)
    {
        const float xfade_time = 0.005f;

        const float xfade_step = 1.0f / (xfade_time * (float)dstate->audio_rate);
        const float xfade_left = 1 - lpstate->xfade_progress;
        int32_t xfade_buf_stop =
            buf_start + (int32_t)ceilf(xfade_left / xfade_step);
        xfade_buf_stop = min(xfade_buf_stop, buf_stop);

        if (xfade_buf_stop > buf_start)
        {
            float* xfade_out_data[2] =
            {
                Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_FADING_OUT_L),
                Work_buffers_get_buffer_contents_mut(wbs, LOOPER_WB_FADING_OUT_R),
            };

            for (int ch = 0; ch < 2; ++ch)
            {
                float* xfade_out = xfade_out_data[ch];
                for (int i = buf_start; i < xfade_buf_stop; ++i)
                    xfade_out[i] = 0;
            }

            Mode_context_update(
                    fading_context,
                    total_offsets,
                    speeds,
                    is_recording,
                    delay_max,
                    lpstate->marker_start,
                    lpstate->marker_stop,
                    buf_start,
                    xfade_buf_stop);

            Mode_context_render(
                    fading_context,
                    xfade_out_data,
                    in_data,
                    history_data,
                    total_offsets,
                    history_buf_size,
                    cur_lpstate_write_pos,
                    lpstate->marker_start,
                    lpstate->marker_stop,
                    buf_start,
                    xfade_buf_stop);

            float xfade_progress = lpstate->xfade_progress;

            for (int ch = 0; ch < 2; ++ch)
            {
                xfade_progress = lpstate->xfade_progress;

                float* out = out_data[ch];
                if (out == NULL)
                    continue;

                const float* xfade_out = xfade_out_data[ch];

                for (int32_t i = buf_start; i < xfade_buf_stop; ++i)
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

#if 0
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
#endif

    if (enable_playback)
    {
#if 0
        for (int ch = 0; ch < 2; ++ch)
        {
            float* in = in_data[ch];
            float* out = out_data[ch];
            if ((in == NULL) || (out == NULL))
                continue;

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
#endif
    }

    if (is_recording)
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

        // Update range markers (for the next update)
        {
            const int32_t frame_count = buf_stop - buf_start;
            if (lpstate->marker_start <= 0)
                lpstate->marker_start -= frame_count;
            if (lpstate->marker_stop <= 0)
                lpstate->marker_stop -= frame_count;
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

    main_context->mode = fading_context->mode;
    main_context->read_pos = fading_context->read_pos;

    lpstate->xfade_progress = 0;

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
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        main_context->mode = MODE_PLAY;
        if (main_context->read_pos >= (float)lpstate->marker_stop)
            main_context->read_pos = (float)lpstate->marker_start;
    }
    else if (string_eq(event_name, "stop"))
    {
        switch_context(lpstate);
        Mode_context* main_context = get_main_context(lpstate);

        main_context->mode = MODE_STOP;
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
    lpstate->main_context = 0;
    lpstate->xfade_progress = 1;
    for (int i = 0; i < 2; ++i)
        Mode_context_init(&lpstate->contexts[i]);
    lpstate->contexts[lpstate->main_context].mode = MODE_RECORD;

    lpstate->marker_start = 1;
    lpstate->marker_stop = 1;
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


