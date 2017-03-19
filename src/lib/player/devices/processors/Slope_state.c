

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Slope_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_slope.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static void clamp_buffer(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    const int32_t const_start = Work_buffer_get_const_start(buffer);
    float* buf = Work_buffer_get_contents_mut(buffer);

    const float bound = 20000000000.0f;

    for (int32_t i = buf_start; i < buf_stop; ++i)
        buf[i] = clamp(buf[i], -bound, bound);

    Work_buffer_set_const_start(buffer, const_start);

    return;
}


static void process(
        const Work_buffer* in_wb,
        Work_buffer* out_wb,
        double smoothing,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        float* inout_prev_value,
        float* inout_prev_slope)
{
    rassert(in_wb != NULL);
    rassert(out_wb != NULL);
    rassert(smoothing >= 0);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);
    rassert(audio_rate > 0);
    rassert(inout_prev_value != NULL);
    rassert(inout_prev_slope != NULL);

    const float* in = Work_buffer_get_contents(in_wb);
    float* out = Work_buffer_get_contents_mut(out_wb);

    const double smoothing_factor = 0.1 / 12.0;
    const float smooth_lerp =
        (float)min(1.0, 1.0 / (audio_rate * smoothing * smoothing_factor));

    float prev_value = *inout_prev_value;
    float prev_slope = *inout_prev_slope;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float in_value = in[i];

        const float imm_slope = (in_value - prev_value) * (float)audio_rate;
        const float cur_slope = lerp(prev_slope, imm_slope, smooth_lerp);
        out[i] = cur_slope;
        prev_slope = cur_slope;

        prev_value = in_value;
    }

    *inout_prev_value = prev_value;
    *inout_prev_slope = prev_slope;

    return;
}


typedef struct Slope_pstate
{
    Proc_state parent;

    bool anything_rendered;
    float prev_value;
    float prev_slope;
} Slope_pstate;


static void del_Slope_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Slope_pstate* spstate = (Slope_pstate*)dstate;
    memory_free(spstate);

    return;
}


static void Slope_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Slope_pstate* spstate = (Slope_pstate*)dstate;
    spstate->anything_rendered = false;
    spstate->prev_value = 0;
    spstate->prev_slope = 0;

    return;
}


enum
{
    PORT_IN_SIGNAL = 0,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_SLOPE = 0,
    PORT_OUT_COUNT
};


static const int SLOPE_WB_FIXED_INPUT = WORK_BUFFER_IMPL_1;
static const int SLOPE_WB_DUMMY_OUTPUT = WORK_BUFFER_IMPL_2;


static void Slope_pstate_render_mixed(
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

    if (buf_start == buf_stop)
        return;

    Slope_pstate* spstate = (Slope_pstate*)dstate;

    Work_buffer* in_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_IN_SIGNAL);
    if (in_wb == NULL)
    {
        Work_buffer* fixed_in_wb =
            Work_buffers_get_buffer_mut(wbs, SLOPE_WB_FIXED_INPUT);
        Work_buffer_clear(fixed_in_wb, buf_start, buf_stop);
        in_wb = fixed_in_wb;
    }
    else
    {
        clamp_buffer(in_wb, buf_start, buf_stop);
    }

    Work_buffer* out_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SLOPE);
    if (out_wb == NULL)
        out_wb = Work_buffers_get_buffer_mut(wbs, SLOPE_WB_DUMMY_OUTPUT);

    if (!spstate->anything_rendered)
    {
        spstate->prev_value = Work_buffer_get_contents(in_wb)[buf_start];
        spstate->anything_rendered = true;
    }

    const Proc_slope* slope = (const Proc_slope*)dstate->device->dimpl;

    process(in_wb,
            out_wb,
            slope->smoothing,
            buf_start,
            buf_stop,
            Device_state_get_audio_rate(dstate),
            &spstate->prev_value,
            &spstate->prev_slope);

    clamp_buffer(out_wb, buf_start, buf_stop);

    return;
}


static void Slope_pstate_clear_history(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    Slope_pstate_reset((Device_state*)proc_state);

    return;
}


Device_state* new_Slope_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Slope_pstate* spstate = memory_alloc_item(Slope_pstate);
    if (spstate == NULL)
        return NULL;

    if (!Proc_state_init(&spstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(spstate);
        return NULL;
    }

    spstate->parent.destroy = del_Slope_pstate;
    spstate->parent.reset = Slope_pstate_reset;
    spstate->parent.render_mixed = Slope_pstate_render_mixed;
    spstate->parent.clear_history = Slope_pstate_clear_history;

    spstate->anything_rendered = false;
    spstate->prev_value = 0;
    spstate->prev_slope = 0;

    return &spstate->parent.parent;
}


typedef struct Slope_vstate
{
    Voice_state parent;

    bool anything_rendered;
    float prev_value;
    float prev_slope;
} Slope_vstate;


int32_t Slope_vstate_get_size(void)
{
    return sizeof(Slope_vstate);
}


static int32_t Slope_vstate_render_voice(
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
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);
    rassert(tempo > 0);

    if (buf_start == buf_stop)
        return buf_start;

    const Device_state* dstate = &proc_state->parent;
    Slope_vstate* svstate = (Slope_vstate*)vstate;

    Work_buffer* in_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL);
    if (in_wb == NULL)
    {
        Work_buffer* fixed_in_wb =
            Work_buffers_get_buffer_mut(wbs, SLOPE_WB_FIXED_INPUT);
        Work_buffer_clear(fixed_in_wb, buf_start, buf_stop);
        in_wb = fixed_in_wb;
    }
    else
    {
        clamp_buffer(in_wb, buf_start, buf_stop);
    }

    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SLOPE);
    if (out_wb == NULL)
        out_wb = Work_buffers_get_buffer_mut(wbs, SLOPE_WB_DUMMY_OUTPUT);

    if (!svstate->anything_rendered)
    {
        svstate->prev_value = Work_buffer_get_contents(in_wb)[buf_start];
        svstate->anything_rendered = true;
    }

    const Proc_slope* slope = (const Proc_slope*)proc_state->parent.device->dimpl;

    process(in_wb,
            out_wb,
            slope->smoothing,
            buf_start,
            buf_stop,
            Device_state_get_audio_rate(dstate),
            &svstate->prev_value,
            &svstate->prev_slope);

    clamp_buffer(out_wb, buf_start, buf_stop);

    return buf_stop;
}


void Slope_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Slope_vstate_render_voice;

    Slope_vstate* svstate = (Slope_vstate*)vstate;

    svstate->anything_rendered = false;
    svstate->prev_value = 0;
    svstate->prev_slope = 0;

    return;
}


