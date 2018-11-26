

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


#include <player/devices/processors/Volume_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_volume.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>
#include <string/key_pattern.h>

#include <stdint.h>
#include <stdlib.h>


void Volume_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 2, 1, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_FORCE,
    PORT_IN_COUNT,

    PORT_IN_AUDIO_COUNT = PORT_IN_AUDIO_R + 1
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int VOLUME_WB_FIXED_VOLUME = WORK_BUFFER_IMPL_1;


static void apply_volume(
        float* in_buffer,
        float* out_buffer,
        Work_buffer* vol_wb,
        const Work_buffers* wbs,
        float global_vol,
        int32_t frame_count)
{
    rassert(in_buffer != NULL);
    rassert(out_buffer != NULL);
    rassert(isfinite(global_vol));
    rassert(frame_count > 0);

    const float global_scale = (float)dB_to_scale(global_vol);

    Work_buffer* scales_wb = Work_buffers_get_buffer_mut(wbs, VOLUME_WB_FIXED_VOLUME, 1);
    Proc_fill_scale_buffer(scales_wb, vol_wb, 0, frame_count);
    const float* scales = Work_buffer_get_contents(scales_wb, 0);

    const float* in = in_buffer;
    float* out = out_buffer;

    for (int32_t i = 0; i < frame_count; ++i)
    {
        const float scale = scales[i] * global_scale;
        *out++ = (*in++) * scale;
        *out++ = (*in++) * scale;
    }

    return;
}


typedef struct Volume_pstate
{
    Proc_state parent;
    double volume;
} Volume_pstate;


bool Volume_pstate_set_volume(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);

    Volume_pstate* vpstate = (Volume_pstate*)dstate;
    vpstate->volume = isfinite(value) ? value : 0.0;

    return true;
}


static void Volume_pstate_render_mixed(
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

    Volume_pstate* vpstate = (Volume_pstate*)dstate;

    // Get control stream
    Work_buffer* vol_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    if ((vol_wb != NULL) && !Work_buffer_is_valid(vol_wb, 0))
        vol_wb = NULL;

    // Get input
    float* in_buf = NULL;
    {
        Work_buffer* in_wb =
            Proc_get_mixed_input_2ch(proc_ts, PORT_IN_AUDIO_L, 0, frame_count);
        if (in_wb != NULL)
            in_buf = Work_buffer_get_contents_mut(in_wb, 0);
    }

    // Get output
    float* out_buf = NULL;
    {
        Work_buffer* out_wb =
            Proc_get_mixed_output_2ch(proc_ts, PORT_OUT_AUDIO_L, 0, frame_count);
        if (out_wb != NULL)
            out_buf = Work_buffer_get_contents_mut(out_wb, 0);
    }

    rassert(out_buf != NULL);

    if (in_buf == NULL)
    {
        for (int32_t i = 0; i < frame_count * 2; ++i)
            out_buf[i] = 0;
        return;
    }

    apply_volume(in_buf, out_buf, vol_wb, wbs, (float)vpstate->volume, frame_count);

    return;
}


Device_state* new_Volume_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Volume_pstate* vol_state = memory_alloc_item(Volume_pstate);
    if ((vol_state == NULL) ||
            !Proc_state_init(&vol_state->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(vol_state);
        return NULL;
    }

    vol_state->parent.render_mixed = Volume_pstate_render_mixed;

    vol_state->volume = 0.0;

    return (Device_state*)vol_state;
}


int32_t Volume_vstate_get_size(void)
{
    return 0;
}


int32_t Volume_vstate_render_voice(
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

    // Get control stream
    Work_buffer* vol_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    if ((vol_wb != NULL) &&
            Work_buffer_is_final(vol_wb, 0) &&
            (Work_buffer_get_const_start(vol_wb, 0) == 0) &&
            (Work_buffer_get_contents(vol_wb, 0)[0] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        return 0;
    }

    // Get input
    float* in_buf = NULL;
    {
        Work_buffer* in_wb =
            Proc_get_voice_input_2ch(proc_ts, PORT_IN_AUDIO_L, 0, frame_count);
        if (in_wb != NULL)
            in_buf = Work_buffer_get_contents_mut(in_wb, 0);
    }

    if (in_buf == NULL)
        return 0;

    // Get output
    float* out_buf = NULL;
    {
        Work_buffer* out_wb =
            Proc_get_voice_output_2ch(proc_ts, PORT_OUT_AUDIO_L, 0, frame_count);
        if (out_wb != NULL)
            out_buf = Work_buffer_get_contents_mut(out_wb, 0);
    }

    rassert(out_buf != NULL);

    const Volume_pstate* vpstate = (const Volume_pstate*)proc_state;
    apply_volume(in_buf, out_buf, vol_wb, wbs, (float)vpstate->volume, frame_count);

    return frame_count;
}


