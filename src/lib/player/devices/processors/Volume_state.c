

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
#include <string/key_pattern.h>

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


static void apply_volume(
        int buf_count,
        float* in_buffers[buf_count],
        float* out_buffers[buf_count],
        Work_buffer* vol_wb,
        float global_vol,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(buf_count > 0);
    rassert(in_buffers != NULL);
    rassert(out_buffers != NULL);
    rassert(isfinite(global_vol));
    rassert(buf_start >= 0);

    const float global_scale = (float)dB_to_scale(global_vol);

    // Copy input to output with global volume adjustment
    for (int ch = 0; ch < buf_count; ++ch)
    {
        const float* in = in_buffers[ch];
        float* out = out_buffers[ch];
        if ((in == NULL) || (out == NULL))
        {
            if (out != NULL)
            {
                for (int32_t i = buf_start; i < buf_stop; ++i)
                    out[i] = 0;
            }

            continue;
        }

        for (int32_t frame = buf_start; frame < buf_stop; ++frame)
            out[frame] = in[frame] * global_scale;
    }

    // Adjust output based on volume buffer
    if (vol_wb != NULL)
    {
        Proc_fill_scale_buffer(vol_wb, vol_wb, buf_start, buf_stop);
        const float* scales = Work_buffer_get_contents(vol_wb, 0);

        for (int ch = 0; ch < buf_count; ++ch)
        {
            float* out = out_buffers[ch];
            if (out == NULL)
                continue;

            for (int32_t i = buf_start; i < buf_stop; ++i)
                out[i] *= scales[i];
        }
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
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Volume_pstate* vpstate = (Volume_pstate*)dstate;

    // Get control stream
    Work_buffer* vol_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    if ((vol_wb != NULL) && !Work_buffer_is_valid(vol_wb, 0))
        vol_wb = NULL;

    // Get input
    float* in_bufs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* in_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch, NULL);
        if ((in_wb != NULL) && Work_buffer_is_valid(in_wb, 0))
            in_bufs[ch] = Work_buffer_get_contents_mut(in_wb, 0);
    }

    // Get output
    float* out_bufs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch, NULL);
        if (out_wb != NULL)
            out_bufs[ch] = Work_buffer_get_contents_mut(out_wb, 0);
    }

    apply_volume(
            2, in_bufs, out_bufs, vol_wb, (float)vpstate->volume, buf_start, buf_stop);

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
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate == NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    // Get control stream
    Work_buffer* vol_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    if ((vol_wb != NULL) &&
            Work_buffer_is_final(vol_wb, 0) &&
            (Work_buffer_get_const_start(vol_wb, 0) <= buf_start) &&
            (Work_buffer_get_contents(vol_wb, 0)[buf_start] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        return buf_start;
    }

    // Get input
    float* in_bufs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* in_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch, NULL);
        if ((in_wb != NULL) && Work_buffer_is_valid(in_wb, 0))
            in_bufs[ch] = Work_buffer_get_contents_mut(in_wb, 0);
    }

    if ((in_bufs[0] == NULL) && (in_bufs[1] == NULL))
        return buf_start;

    // Get output
    float* out_bufs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch, NULL);
        if (out_wb != NULL)
            out_bufs[ch] = Work_buffer_get_contents_mut(out_wb, 0);
    }

    const Volume_pstate* vpstate = (const Volume_pstate*)proc_state;
    apply_volume(
            2,
            in_bufs,
            out_bufs,
            vol_wb,
            (float)vpstate->volume,
            buf_start,
            buf_stop);

    return buf_stop;
}


