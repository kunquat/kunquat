

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2020
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Rangemap_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/param_types/Envelope.h>
#include <init/devices/processors/Proc_rangemap.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdlib.h>


static void get_linear_scalars(
        float* mul,
        float* add,
        double from_min,
        double from_max,
        double min_to,
        double max_to)
{
    rassert(mul != NULL);
    rassert(add != NULL);
    rassert(isfinite(from_min));
    rassert(isfinite(from_max));
    rassert(isfinite(min_to));
    rassert(isfinite(max_to));

    if (from_max <= from_min || min_to == max_to)
    {
        *mul = 0;
        *add = (float)min_to;
        return;
    }

    const double from_range = from_max - from_min;
    const double to_range = max_to - min_to;
    *mul = (float)(to_range / from_range);
    *add = (float)(min_to - (*mul * from_min));

    return;
}


static void get_extrapolated_envelope_scalars(
        float* mul,
        float* add,
        bool clamp_dest_min,
        float* mul2,
        float* add2,
        bool clamp_dest_max,
        const Envelope* envelope)
{
    rassert(mul != NULL);
    rassert(add != NULL);
    rassert(mul2 != NULL);
    rassert(add2 != NULL);
    rassert(envelope != NULL);
    rassert(Envelope_node_count(envelope) >= 2);

    if (clamp_dest_min)
    {
        *mul = 0;
        *add = (float)Envelope_get_node(envelope, 0)[1];
    }
    else
    {
        const double* node1 = Envelope_get_node(envelope, 0);
        const double* node2 = Envelope_get_node(envelope, 1);
        get_linear_scalars(mul, add, node1[0], node2[0], node1[1], node2[1]);
    }

    if (clamp_dest_max)
    {
        *mul2 = 0;
        *add2 = (float)Envelope_get_node(envelope, Envelope_node_count(envelope) - 1)[1];
    }
    else
    {
        const double* node1 =
            Envelope_get_node(envelope, Envelope_node_count(envelope) - 2);
        const double* node2 =
            Envelope_get_node(envelope, Envelope_node_count(envelope) - 1);
        get_linear_scalars(mul2, add2, node1[0], node2[0], node1[1], node2[1]);
    }

    return;
}


static void get_scalars(
        float* mul, float* add, float* mul2, float* add2, const Proc_rangemap* rangemap)
{
    rassert(mul != NULL);
    rassert(add != NULL);
    rassert(mul2 != NULL);
    rassert(add2 != NULL);

    if (rangemap->is_env_enabled && (rangemap->envelope != NULL))
    {
        get_extrapolated_envelope_scalars(
                mul,
                add,
                rangemap->clamp_dest_min,
                mul2,
                add2,
                rangemap->clamp_dest_max,
                rangemap->envelope);
    }
    else
    {
        get_linear_scalars(
                mul,
                add,
                rangemap->from_min, rangemap->from_max,
                rangemap->min_to, rangemap->max_to);
    }

    return;
}


static void apply_linear_range(
        const Work_buffer* in_wb,
        Work_buffer* out_wb,
        int32_t frame_count,
        float mul,
        float add,
        float min_val,
        float max_val)
{
    rassert(in_wb != NULL);
    rassert(out_wb != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(mul));
    rassert(isfinite(add));
    rassert(min_val <= max_val);

    const float* in = Work_buffer_get_contents(in_wb);
    float* out = Work_buffer_get_contents_mut(out_wb);

    for (int32_t i = 0; i < frame_count; ++i)
        out[i] = (mul * in[i]) + add;

    if (isfinite(min_val))
    {
        for (int32_t i = 0; i < frame_count; ++i)
            out[i] = max(out[i], min_val);
    }

    if (isfinite(max_val))
    {
        for (int32_t i = 0; i < frame_count; ++i)
            out[i] = min(out[i], max_val);
    }

    return;
}


static void apply_envelope_range(
        const Work_buffer* in_wb,
        Work_buffer* out_wb,
        int32_t frame_count,
        float mul,
        float add,
        float mul2,
        float add2,
        const Envelope* envelope)
{
    rassert(in_wb != NULL);
    rassert(out_wb != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(mul));
    rassert(isfinite(add));
    rassert(isfinite(mul2));
    rassert(isfinite(add2));
    rassert(envelope != NULL);

    const float env_min = (float)Envelope_get_node(envelope, 0)[0];
    const float env_max =
        (float)Envelope_get_node(envelope, Envelope_node_count(envelope) - 1)[0];

    const float* in = Work_buffer_get_contents(in_wb);
    float* out = Work_buffer_get_contents_mut(out_wb);

    for (int32_t i = 0; i < frame_count; ++i)
    {
        const float in_val = in[i];

        if (in_val < env_min)
            out[i] = (mul * in_val) + add;
        else if (in_val > env_max)
            out[i] = (mul2 * in_val) + add2;
        else
            out[i] = (float)Envelope_get_value(envelope, in_val);
    }

    return;
}


static const int RANGEMAP_WB_FIXED_INPUT = WORK_BUFFER_IMPL_1;


static void Rangemap_pstate_render_mixed(
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

    const Proc_rangemap* rangemap = (const Proc_rangemap*)dstate->device->dimpl;

    float mul = 0;
    float add = 0;
    float mul2 = 0;
    float add2 = 0;
    get_scalars(&mul, &add, &mul2, &add2, rangemap);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    const bool use_envelope = (rangemap->is_env_enabled && (rangemap->envelope != NULL));

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb =
            Device_thread_state_get_mixed_buffer(proc_ts, DEVICE_PORT_TYPE_SEND, ch);
        if (out_wb == NULL)
            continue;

        Work_buffer* in_wb =
            Device_thread_state_get_mixed_buffer(proc_ts, DEVICE_PORT_TYPE_RECV, ch);
        if (!Work_buffer_is_valid(in_wb))
        {
            in_wb = Work_buffers_get_buffer_mut(wbs, RANGEMAP_WB_FIXED_INPUT);
            Work_buffer_clear(in_wb, 0, frame_count);
        }

        if (!use_envelope)
            apply_linear_range(in_wb, out_wb, frame_count, mul, add, min_val, max_val);
        else
            apply_envelope_range(
                    in_wb,
                    out_wb,
                    frame_count,
                    mul,
                    add,
                    mul2,
                    add2,
                    rangemap->envelope);

        const int32_t const_start = Work_buffer_get_const_start(in_wb);
        Work_buffer_set_const_start(out_wb, const_start);
    }

    return;
}


Device_state* new_Rangemap_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_mixed = Rangemap_pstate_render_mixed;

    return (Device_state*)proc_state;
}


int32_t Rangemap_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate == NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_rangemap* rangemap =
        (const Proc_rangemap*)proc_state->parent.device->dimpl;

    float mul = 0;
    float add = 0;
    float mul2 = 0;
    float add2 = 0;
    get_scalars(&mul, &add, &mul2, &add2, rangemap);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    const bool use_envelope = (rangemap->is_env_enabled && (rangemap->envelope != NULL));

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb =
            Device_thread_state_get_voice_buffer(proc_ts, DEVICE_PORT_TYPE_SEND, ch);
        if (out_wb == NULL)
            continue;

        Work_buffer* in_wb =
            Device_thread_state_get_voice_buffer(proc_ts, DEVICE_PORT_TYPE_RECV, ch);
        if (!Work_buffer_is_valid(in_wb))
        {
            in_wb = Work_buffers_get_buffer_mut(wbs, RANGEMAP_WB_FIXED_INPUT);
            Work_buffer_clear(in_wb, 0, frame_count);
        }

        if (!use_envelope)
            apply_linear_range(in_wb, out_wb, frame_count, mul, add, min_val, max_val);
        else
            apply_envelope_range(
                    in_wb,
                    out_wb,
                    frame_count,
                    mul,
                    add,
                    mul2,
                    add2,
                    rangemap->envelope);

        const int32_t const_start = Work_buffer_get_const_start(in_wb);
        const bool is_final = Work_buffer_is_final(in_wb);

        Work_buffer_set_const_start(out_wb, const_start);
        Work_buffer_set_final(out_wb, is_final);
    }

    return frame_count;
}


int32_t Rangemap_vstate_get_size(void)
{
    return 0;
}


