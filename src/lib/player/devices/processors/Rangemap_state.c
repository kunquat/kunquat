

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
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
#include <init/devices/processors/Proc_rangemap.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdlib.h>


void Rangemap_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);
    rassert(groups != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 2, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


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

    const float* in = Work_buffer_get_contents(in_wb, 0);
    float* out = Work_buffer_get_contents_mut(out_wb, 0);

    const int32_t item_count = frame_count * 2;

    for (int32_t i = 0; i < item_count; ++i)
        out[i] = (mul * in[i]) + add;

    if (isfinite(min_val))
    {
        for (int32_t i = 0; i < item_count; ++i)
            out[i] = max(out[i], min_val);
    }

    if (isfinite(max_val))
    {
        for (int32_t i = 0; i < item_count; ++i)
            out[i] = min(out[i], max_val);
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
    get_scalars(
            &mul,
            &add,
            rangemap->from_min, rangemap->from_max,
            rangemap->min_to, rangemap->max_to);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    Work_buffer* in_wb = Proc_get_mixed_input_2ch(proc_ts, 0, frame_count);
    if (in_wb == NULL)
    {
        in_wb = Work_buffers_get_buffer_mut(wbs, RANGEMAP_WB_FIXED_INPUT, 2);
        Work_buffer_clear_all(in_wb, 0, frame_count);
    }

    Work_buffer* out_wb = Proc_get_mixed_output_2ch(proc_ts, 0);
    rassert(out_wb != NULL);

    apply_range(in_wb, out_wb, frame_count, mul, add, min_val, max_val);

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
    get_scalars(
            &mul,
            &add,
            rangemap->from_min, rangemap->from_max,
            rangemap->min_to, rangemap->max_to);

    const double range_min = min(rangemap->min_to, rangemap->max_to);
    const double range_max = max(rangemap->min_to, rangemap->max_to);

    const float min_val = (float)(rangemap->clamp_dest_min ? range_min : -INFINITY);
    const float max_val = (float)(rangemap->clamp_dest_max ? range_max : INFINITY);

    Work_buffer* in_wb = Proc_get_voice_input_2ch(proc_ts, 0, frame_count);
    if (in_wb == NULL)
    {
        in_wb = Work_buffers_get_buffer_mut(wbs, RANGEMAP_WB_FIXED_INPUT, 2);
        Work_buffer_clear_all(in_wb, 0, frame_count);
    }

    Work_buffer* out_wb = Proc_get_voice_output_2ch(proc_ts, 0);
    rassert(out_wb != NULL);

    apply_range(in_wb, out_wb, frame_count, mul, add, min_val, max_val);

    return frame_count;
}


int32_t Rangemap_vstate_get_size(void)
{
    return 0;
}


