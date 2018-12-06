

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


void Delay_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);
    rassert(groups != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 2, 1, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


typedef struct Delay_pstate
{
    Proc_state parent;

    Work_buffer* buf;
    int32_t buf_pos;
} Delay_pstate;


static void del_Delay_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    del_Work_buffer(dpstate->buf);
    dpstate->buf = NULL;

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

    if (!Work_buffer_resize(dpstate->buf, delay_buf_size))
        return false;
    Work_buffer_clear(dpstate->buf, 0, 0, Work_buffer_get_size(dpstate->buf));

    dpstate->buf_pos = 0;

    return true;
}


static void Delay_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    Work_buffer_clear_all(dpstate->buf, 0, Work_buffer_get_size(dpstate->buf));

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


static const int DELAY_WB_FIXED_INPUT = WORK_BUFFER_IMPL_1;
static const int DELAY_WB_TOTAL_OFFSETS = WORK_BUFFER_IMPL_2;
static const int DELAY_WB_FIXED_DELAY = WORK_BUFFER_IMPL_3;


static void Delay_pstate_render_mixed(
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
    rassert(tempo > 0);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    int stride = 2;
    float* in_buf = NULL;
    {
        Work_buffer* in_wb =
            Proc_get_mixed_input_2ch(proc_ts, PORT_IN_AUDIO_L, frame_count);
        if (in_wb == NULL)
        {
            in_wb = Work_buffers_get_buffer_mut(wbs, DELAY_WB_FIXED_INPUT, 2);
            Work_buffer_clear_all(in_wb, 0, frame_count);
        }

        rassert(Work_buffer_get_stride(in_wb) == stride);

        in_buf = Work_buffer_get_contents_mut(in_wb, 0);
    }

    float* out_buf = NULL;
    {
        Work_buffer* out_wb = Proc_get_mixed_output_2ch(proc_ts, PORT_OUT_AUDIO_L);
        rassert(out_wb != NULL);
        out_buf = Work_buffer_get_contents_mut(out_wb, 0);
    }

    float* history_data = Work_buffer_get_contents_mut(dpstate->buf, 0);

    const int32_t delay_buf_size = Work_buffer_get_size(dpstate->buf);
    const int32_t delay_max = delay_buf_size - 1;

    float* total_offsets =
        Work_buffers_get_buffer_contents_mut(wbs, DELAY_WB_TOTAL_OFFSETS);

    // Get delay stream
    Work_buffer* delays_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_DELAY, NULL);
    float* delays = ((delays_wb != NULL) && Work_buffer_is_valid(delays_wb, 0))
            ? Work_buffer_get_contents_mut(delays_wb, 0) : NULL;
    if (delays == NULL)
    {
        delays = Work_buffers_get_buffer_contents_mut(wbs, DELAY_WB_FIXED_DELAY);
        const float init_delay = (float)delay->init_delay;
        for (int32_t i = 0; i < frame_count; ++i)
            delays[i] = init_delay;
    }

    int32_t cur_dpstate_buf_pos = dpstate->buf_pos;

    const int32_t audio_rate = dstate->audio_rate;

    // Get total offsets
    for (int32_t i = 0, chunk_offset = 0; i < frame_count; ++i, ++chunk_offset)
    {
        const float cur_delay = delays[i];
        double delay_frames = cur_delay * (double)audio_rate;
        delay_frames = clamp(delay_frames, 0, delay_max);
        total_offsets[i] = (float)(chunk_offset - delay_frames);
    }

    // Clamp input values to finite range
    // (this is required due to possible multiplication by zero below)
    {
        const int32_t item_count = frame_count * 2;
        for (int32_t i = 0; i < item_count; ++i)
            in_buf[i] = clamp(in_buf[i], -FLT_MAX, FLT_MAX);
    }

    for (int32_t i = 0; i < frame_count; ++i)
    {
        const float total_offset = total_offsets[i];

        // Get buffer positions
        const int32_t cur_pos = (int32_t)floor(total_offset);
        const float remainder = total_offset - (float)cur_pos;
        rassert(cur_pos <= (int32_t)i);
        rassert(implies(cur_pos == (int32_t)i, remainder == 0));
        const int32_t next_pos = cur_pos + 1;

        // Get audio frames
        float cur_val_l = 0;
        float cur_val_r = 0;
        float next_val_l = 0;
        float next_val_r = 0;

        if (cur_pos >= 0)
        {
            rassert(cur_pos < frame_count);
            cur_val_l = in_buf[cur_pos * stride];
            cur_val_r = in_buf[cur_pos * stride + 1];

            const int32_t in_next_pos = min(next_pos, i);
            next_val_l = in_buf[in_next_pos * stride];
            next_val_r = in_buf[in_next_pos * stride + 1];
        }
        else
        {
            const int32_t cur_delay_buf_pos =
                (cur_dpstate_buf_pos + cur_pos + delay_buf_size) % delay_buf_size;
            rassert(cur_delay_buf_pos >= 0);

            cur_val_l = history_data[cur_delay_buf_pos * stride];
            cur_val_r = history_data[cur_delay_buf_pos * stride + 1];

            if (next_pos < 0)
            {
                const int32_t next_delay_buf_pos =
                    (cur_dpstate_buf_pos + next_pos + delay_buf_size) % delay_buf_size;
                rassert(next_delay_buf_pos >= 0);

                next_val_l = history_data[next_delay_buf_pos * stride];
                next_val_r = history_data[next_delay_buf_pos * stride + 1];
            }
            else
            {
                rassert(next_pos == 0);
                next_val_l = in_buf[0];
                next_val_r = in_buf[1];
            }
        }

        // Create output frame
        out_buf[i * stride] = lerp(cur_val_l, next_val_l, remainder);
        out_buf[i * stride + 1] = lerp(cur_val_r, next_val_r, remainder);
    }

    for (int32_t i = 0; i < frame_count; ++i)
    {
        history_data[cur_dpstate_buf_pos * stride] = in_buf[i * stride];
        history_data[cur_dpstate_buf_pos * stride + 1] = in_buf[i * stride + 1];

        ++cur_dpstate_buf_pos;
        if (cur_dpstate_buf_pos >= delay_buf_size)
        {
            rassert(cur_dpstate_buf_pos == delay_buf_size);
            cur_dpstate_buf_pos = 0;
        }
    }

    dpstate->buf_pos = cur_dpstate_buf_pos;

    return;
}


static void Delay_pstate_clear_history(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)proc_state;

    Work_buffer_clear(dpstate->buf, 0, 0, Work_buffer_get_size(dpstate->buf));

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

    dpstate->buf = NULL;

    const Proc_delay* delay = (const Proc_delay*)device->dimpl;

    const int32_t delay_buf_size = (int32_t)(delay->max_delay * audio_rate + 1);

    dpstate->buf = new_Work_buffer(delay_buf_size, 2);
    if (dpstate->buf == NULL)
    {
        del_Device_state(&dpstate->parent.parent);
        return NULL;
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

    if (!Work_buffer_resize(dpstate->buf, delay_buf_size))
        return false;
    Work_buffer_clear_all(dpstate->buf, 0, Work_buffer_get_size(dpstate->buf));

    dpstate->buf_pos = 0;

    return true;
}


