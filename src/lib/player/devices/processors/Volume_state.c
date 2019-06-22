

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
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
#include <mathnum/common.h>
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

#include <float.h>
#include <stdint.h>
#include <stdlib.h>


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
        const Work_buffer* in_wbs[2],
        Work_buffer* out_wbs[2],
        Work_buffer* vol_wb,
        const Work_buffers* wbs,
        float global_vol,
        int32_t frame_count)
{
    rassert(in_wbs != NULL);
    rassert(out_wbs != NULL);
    rassert(isfinite(global_vol));
    rassert(frame_count > 0);

    const float global_scale = (float)dB_to_scale(global_vol);

    int32_t vol_const_start = 0;
    bool is_vol_final = true;
    if (Work_buffer_is_valid(vol_wb))
    {
        vol_const_start = Work_buffer_get_const_start(vol_wb);
        is_vol_final = Work_buffer_is_final(vol_wb);
    }

    Work_buffer* scales_wb = Work_buffers_get_buffer_mut(wbs, VOLUME_WB_FIXED_VOLUME);
    Proc_fill_scale_buffer(scales_wb, vol_wb, frame_count);
    const float* scales = Work_buffer_get_contents(scales_wb);

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = out_wbs[ch];
        if (out_wb == NULL)
            continue;

        const Work_buffer* in_wb = in_wbs[ch];
        if (!Work_buffer_is_valid(in_wb))
            continue;

        const int32_t in_const_start = Work_buffer_get_const_start(in_wb);
        const bool is_in_final = Work_buffer_is_final(in_wb);

        const float* in = Work_buffer_get_contents(in_wb);
        float* out = Work_buffer_get_contents_mut(out_wb);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float scale = scales[i] * global_scale;
            float in_clamped = *in++;
            in_clamped = clamp(in_clamped, -FLT_MAX, FLT_MAX);
            *out++ = in_clamped * scale;
        }

        const int32_t out_const_start = max(vol_const_start, in_const_start);
        const bool is_out_final = is_vol_final && is_in_final;

        Work_buffer_set_const_start(out_wb, out_const_start);
        Work_buffer_set_final(out_wb, is_out_final);
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
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE);
    if ((vol_wb != NULL) && !Work_buffer_is_valid(vol_wb))
        vol_wb = NULL;

    // Get inputs
    const Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        in_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);

    // Get outputs
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);

    apply_volume(in_wbs, out_wbs, vol_wb, wbs, (float)vpstate->volume, frame_count);

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
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE);
    if (Work_buffer_is_valid(vol_wb) &&
            Work_buffer_is_final(vol_wb) &&
            (Work_buffer_get_const_start(vol_wb) == 0) &&
            (Work_buffer_get_contents(vol_wb)[0] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        return 0;
    }

    // Get input
    const Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        in_wbs[ch] = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);

    if (!Work_buffer_is_valid(in_wbs[0]) && !Work_buffer_is_valid(in_wbs[1]))
        return 0;

    // Get output
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);

    const Volume_pstate* vpstate = (const Volume_pstate*)proc_state;
    apply_volume(in_wbs, out_wbs, vol_wb, wbs, (float)vpstate->volume, frame_count);

    return frame_count;
}


