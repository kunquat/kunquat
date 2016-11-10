

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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
#include <init/devices/processors/Proc_rangemap.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>

#include <stdint.h>
#include <stdlib.h>


static void get_scalars(
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


static void apply_range(
        const Work_buffer* in_wb,
        Work_buffer* out_wb,
        int32_t buf_start,
        int32_t buf_stop,
        float mul,
        float add,
        float min_val,
        float max_val)
{
    rassert(in_wb != NULL);
    rassert(out_wb != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);
    rassert(isfinite(mul));
    rassert(isfinite(add));
    rassert(min_val < max_val);

    const float* in = Work_buffer_get_contents(in_wb);
    float* out = Work_buffer_get_contents_mut(out_wb);

    for (int32_t i = buf_start; i < buf_stop; ++i)
        out[i] = (mul * in[i]) + add;

    if (isfinite(min_val))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out[i] = max(out[i], min_val);
    }

    if (isfinite(max_val))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out[i] = min(out[i], max_val);
    }

    return;
}


static void Rangemap_pstate_render_mixed(
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
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_rangemap* rangemap = (const Proc_rangemap*)dstate->device->dimpl;

    float mul = 0;
    float add = 0;
    get_scalars(
            &mul,
            &add,
            rangemap->from_min, rangemap->from_max,
            rangemap->min_to, rangemap->max_to);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    // TODO: Support all ports?
    //       We should only enable the support if we are 100% sure that
    //       we don't need parameter input streams
    for (int port = 0; port < 2; ++port)
    {
        Work_buffer* out_wb =
            Device_thread_state_get_mixed_buffer(proc_ts, DEVICE_PORT_TYPE_SEND, port);
        if (out_wb == NULL)
            continue;

        const Work_buffer* in_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, port);
        if (in_wb == NULL)
            continue;

        apply_range(in_wb, out_wb, buf_start, buf_stop, mul, add, min_val, max_val);
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


static int32_t Rangemap_vstate_render_voice(
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
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_rangemap* rangemap =
        (const Proc_rangemap*)proc_state->parent.device->dimpl;

    float mul = 0;
    float add = 0;
    get_scalars(
            &mul,
            &add,
            rangemap->from_min, rangemap->from_max,
            rangemap->min_to, rangemap->max_to);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    // TODO: Support all ports?
    //       We should only enable the support if we are 100% sure that
    //       we don't need parameter input streams
    for (int port = 0; port < 2; ++port)
    {
        Work_buffer* out_wb = Proc_state_get_voice_buffer_mut(
                proc_state, proc_ts, DEVICE_PORT_TYPE_SEND, port);
        if (out_wb == NULL)
            continue;

        const Work_buffer* in_wb = Proc_state_get_voice_buffer(
                proc_state, proc_ts, DEVICE_PORT_TYPE_RECV, port);
        if (in_wb == NULL)
            continue;

        apply_range(in_wb, out_wb, buf_start, buf_stop, mul, add, min_val, max_val);
    }

    return buf_stop;
}


void Rangemap_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Rangemap_vstate_render_voice;

    return;
}


