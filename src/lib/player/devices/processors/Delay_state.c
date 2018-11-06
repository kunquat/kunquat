

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


#include <player/devices/processors/Delay_state.h>

#include <init/devices/processors/Proc_delay.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Linear_controls.h>
#include <player/Player.h>
#include <player/Work_buffer.h>

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Delay_pstate
{
    Proc_state parent;

    Work_buffer* bufs[KQT_BUFFERS_MAX];
    int32_t buf_pos;
} Delay_pstate;


static void del_Delay_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        if (dpstate->bufs[i] != NULL)
        {
            del_Work_buffer(dpstate->bufs[i]);
            dpstate->bufs[i] = NULL;
        }
    }

    memory_free(dpstate);

    return;
}


static bool Delay_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    rassert(dstate != NULL);
    rassert(audio_rate > 0);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    const int32_t delay_buf_size = (int32_t)(delay->max_delay * audio_rate + 1);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = dpstate->bufs[i];

        if (!Work_buffer_resize(buf, delay_buf_size))
            return false;

        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
    }

    dpstate->buf_pos = 0;

    return true;
}


static void Delay_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = dpstate->bufs[i];
        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
    }

    dpstate->buf_pos = 0;

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_DELAY,
    PORT_IN_COUNT,

    PORT_IN_AUDIO_COUNT = PORT_IN_AUDIO_R + 1
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int DELAY_WORK_BUFFER_TOTAL_OFFSETS = WORK_BUFFER_IMPL_1;
static const int DELAY_WORK_BUFFER_FIXED_DELAY = WORK_BUFFER_IMPL_2;


static void Delay_pstate_render_mixed(
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
    rassert(buf_start <= buf_stop);
    rassert(tempo > 0);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    float* in_data[2] = { NULL };
    Proc_state_get_mixed_audio_in_buffers(
            proc_ts, PORT_IN_AUDIO_L, PORT_IN_AUDIO_COUNT, in_data);

    float* out_data[2] = { NULL };
    Proc_state_get_mixed_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_data);

    float* history_data[] =
    {
        Work_buffer_get_contents_mut(dpstate->bufs[0]),
        Work_buffer_get_contents_mut(dpstate->bufs[1]),
    };

    const int32_t delay_buf_size = Work_buffer_get_size(dpstate->bufs[0]);
    rassert(delay_buf_size == Work_buffer_get_size(dpstate->bufs[1]));
    const int32_t delay_max = delay_buf_size - 1;

    float* total_offsets = Work_buffers_get_buffer_contents_mut(
            wbs, DELAY_WORK_BUFFER_TOTAL_OFFSETS);

    // Get delay stream
    float* delays = Device_thread_state_get_mixed_buffer_contents_mut(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_DELAY);
    if (delays == NULL)
    {
        delays =
            Work_buffers_get_buffer_contents_mut(wbs, DELAY_WORK_BUFFER_FIXED_DELAY);
        const float init_delay = (float)delay->init_delay;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            delays[i] = init_delay;
    }

    int32_t cur_dpstate_buf_pos = dpstate->buf_pos;

    const int32_t audio_rate = dstate->audio_rate;

    // Get total offsets
    for (int32_t i = buf_start, chunk_offset = 0; i < buf_stop; ++i, ++chunk_offset)
    {
        const float cur_delay = delays[i];
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

        // Clamp input values to finite range
        // (this is required due to possible multiplication by zero below)
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
                const int32_t cur_delay_buf_pos =
                    (cur_dpstate_buf_pos + cur_pos + delay_buf_size) % delay_buf_size;
                rassert(cur_delay_buf_pos >= 0);

                cur_val = history[cur_delay_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_delay_buf_pos =
                        (cur_dpstate_buf_pos + next_pos + delay_buf_size) %
                        delay_buf_size;
                    rassert(next_delay_buf_pos >= 0);

                    next_val = history[next_delay_buf_pos];
                }
                else
                {
                    rassert(next_pos == 0);
                    next_val = in[buf_start];
                }
            }

            // Create output frame
            const double prev_scale = 1 - remainder;
            const double val =
                (prev_scale * cur_val) + (remainder * next_val);

            out[i] = (float)val;
        }
    }

    // Update the delay state buffers
    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_data[ch];
        if (in == NULL)
            continue;

        float* history = history_data[ch];
        rassert(history != NULL);

        cur_dpstate_buf_pos = dpstate->buf_pos;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            history[cur_dpstate_buf_pos] = in[i];

            ++cur_dpstate_buf_pos;
            if (cur_dpstate_buf_pos >= delay_buf_size)
            {
                rassert(cur_dpstate_buf_pos == delay_buf_size);
                cur_dpstate_buf_pos = 0;
            }
        }
    }

    dpstate->buf_pos = cur_dpstate_buf_pos;

    return;
}


static void Delay_pstate_clear_history(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)proc_state;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = dpstate->bufs[i];
        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
    }

    dpstate->buf_pos = 0;

    return;
}


Device_state* new_Delay_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Delay_pstate* dpstate = memory_alloc_item(Delay_pstate);
    if (dpstate == NULL)
        return NULL;

    if (!Proc_state_init(&dpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(dpstate);
        return NULL;
    }

    dpstate->parent.destroy = del_Delay_pstate;
    dpstate->parent.set_audio_rate = Delay_pstate_set_audio_rate;
    dpstate->parent.reset = Delay_pstate_reset;
    dpstate->parent.render_mixed = Delay_pstate_render_mixed;
    dpstate->parent.clear_history = Delay_pstate_clear_history;
    dpstate->buf_pos = 0;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        dpstate->bufs[i] = NULL;

    const Proc_delay* delay = (const Proc_delay*)device->dimpl;

    const int32_t delay_buf_size = (int32_t)(delay->max_delay * audio_rate + 1);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        dpstate->bufs[i] = new_Work_buffer(delay_buf_size);
        if (dpstate->bufs[i] == NULL)
        {
            del_Device_state(&dpstate->parent.parent);
            return NULL;
        }
    }

    return &dpstate->parent.parent;
}


bool Delay_pstate_set_max_delay(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    ignore(indices);
    ignore(value);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    const int32_t delay_buf_size = (int32_t)(delay->max_delay * dstate->audio_rate + 1);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        Work_buffer* buf = dpstate->bufs[i];

        if (!Work_buffer_resize(buf, delay_buf_size))
            return false;

        Work_buffer_clear(buf, 0, Work_buffer_get_size(buf));
    }

    dpstate->buf_pos = 0;

    return true;
}


