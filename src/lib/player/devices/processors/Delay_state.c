

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


#include <player/devices/processors/Delay_state.h>

#include <init/devices/processors/Proc_delay.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/Audio_buffer.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/Linear_controls.h>
#include <player/Player.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Delay_pstate
{
    Proc_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
} Delay_pstate;


static void del_Delay_pstate(Device_state* dstate)
{
    assert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;
    if (dpstate->buf != NULL)
    {
        del_Audio_buffer(dpstate->buf);
        dpstate->buf = NULL;
    }

    memory_free(dpstate);

    return;
}


static bool Delay_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    const int32_t delay_buf_size = delay->max_delay * audio_rate + 1;

    assert(dpstate->buf != NULL);
    if (!Audio_buffer_resize(dpstate->buf, delay_buf_size))
        return false;

    Audio_buffer_clear(dpstate->buf, 0, Audio_buffer_get_size(dpstate->buf));
    dpstate->buf_pos = 0;

    return true;
}


static void Delay_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const uint32_t delay_buf_size = Audio_buffer_get_size(dpstate->buf);
    Audio_buffer_clear(dpstate->buf, 0, delay_buf_size);
    dpstate->buf_pos = 0;

    return;
}


static void Delay_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start <= buf_stop);
    assert(tempo > 0);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    const float* in_data[] =
    {
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 1),
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 2),
    };

    float* out_data[] =
    {
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_SEND, 0),
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_SEND, 1),
    };

    float* history_data[] =
    {
        Audio_buffer_get_buffer(dpstate->buf, 0),
        Audio_buffer_get_buffer(dpstate->buf, 1),
    };

    const int32_t delay_buf_size = Audio_buffer_get_size(dpstate->buf);
    const int32_t delay_max = delay_buf_size - 1;

    static const int DELAY_WORK_BUFFER_TOTAL_OFFSETS = WORK_BUFFER_IMPL_1;
    static const int DELAY_WORK_BUFFER_FIXED_DELAY = WORK_BUFFER_IMPL_2;

    float* total_offsets = Work_buffers_get_buffer_contents_mut(
            wbs, DELAY_WORK_BUFFER_TOTAL_OFFSETS);

    // Get delay stream
    float* delays =
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (delays == NULL)
    {
        delays =
            Work_buffers_get_buffer_contents_mut(wbs, DELAY_WORK_BUFFER_FIXED_DELAY);
        const float init_delay = delay->init_delay;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            delays[i] = init_delay;
    }

    int32_t cur_dpstate_buf_pos = dpstate->buf_pos;

    const int32_t audio_rate = dstate->audio_rate;

    // Get total offsets
    for (int32_t i = buf_start, chunk_offset = 0; i < buf_stop; ++i, ++chunk_offset)
    {
        const float delay = delays[i];
        float delay_frames = delay * audio_rate;
        delay_frames = clamp(delay_frames, 0, delay_max);
        total_offsets[i] = chunk_offset - delay_frames;
    }

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_data[ch];
        float* out = out_data[ch];
        if ((in == NULL) || (out == NULL))
            continue;

        const float* history = history_data[ch];
        assert(history != NULL);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float total_offset = total_offsets[i];

            // Get buffer positions
            const int32_t cur_pos = (int32_t)floor(total_offset);
            const double remainder = total_offset - cur_pos;
            assert(cur_pos <= (int32_t)i);
            assert(implies(cur_pos == (int32_t)i, remainder == 0));
            const int32_t next_pos = cur_pos + 1;

            // Get audio frames
            double cur_val = 0;
            double next_val = 0;

            if (cur_pos >= 0)
            {
                const int32_t in_cur_pos = buf_start + cur_pos;
                assert(in_cur_pos < (int32_t)buf_stop);
                cur_val = in[in_cur_pos];

                const int32_t in_next_pos = min(buf_start + next_pos, i);
                assert(in_next_pos < (int32_t)buf_stop);
                next_val = in[in_next_pos];
            }
            else
            {
                const int32_t cur_delay_buf_pos =
                    (cur_dpstate_buf_pos + cur_pos + delay_buf_size) % delay_buf_size;
                assert(cur_delay_buf_pos >= 0);

                cur_val = history[cur_delay_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_delay_buf_pos =
                        (cur_dpstate_buf_pos + next_pos + delay_buf_size) %
                        delay_buf_size;
                    assert(next_delay_buf_pos >= 0);

                    next_val = history[next_delay_buf_pos];
                }
                else
                {
                    assert(next_pos == 0);
                    next_val = in[buf_start];
                }
            }

            // Create output frame
            const double prev_scale = 1 - remainder;
            const float val =
                (prev_scale * cur_val) + (remainder * next_val);

            out[i] = val;
        }
    }

    // Update the delay state buffers
    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = in_data[ch];
        if (in == NULL)
            continue;

        float* history = history_data[ch];
        assert(history != NULL);

        cur_dpstate_buf_pos = dpstate->buf_pos;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            history[cur_dpstate_buf_pos] = in[i];

            ++cur_dpstate_buf_pos;
            if (cur_dpstate_buf_pos >= delay_buf_size)
            {
                assert(cur_dpstate_buf_pos == delay_buf_size);
                cur_dpstate_buf_pos = 0;
            }
        }
    }

    dpstate->buf_pos = cur_dpstate_buf_pos;

    return;
}


static void Delay_pstate_clear_history(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Delay_pstate* dpstate = (Delay_pstate*)proc_state;
    Audio_buffer_clear(dpstate->buf, 0, Audio_buffer_get_size(dpstate->buf));

    dpstate->buf_pos = 0;

    return;
}


Device_state* new_Delay_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

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
    dpstate->buf = NULL;
    dpstate->buf_pos = 0;

    const Proc_delay* delay = (const Proc_delay*)device->dimpl;

    const int32_t delay_buf_size = delay->max_delay * audio_rate + 1;

    dpstate->buf = new_Audio_buffer(delay_buf_size);
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
    assert(dstate != NULL);
    ignore(indices);
    ignore(value);

    Delay_pstate* dpstate = (Delay_pstate*)dstate;

    const Proc_delay* delay = (const Proc_delay*)dstate->device->dimpl;

    const int32_t delay_buf_size = delay->max_delay * dstate->audio_rate + 1;

    if (!Audio_buffer_resize(dpstate->buf, delay_buf_size))
        return false;

    Audio_buffer_clear(dpstate->buf, 0, Audio_buffer_get_size(dpstate->buf));
    dpstate->buf_pos = 0;

    return true;
}


