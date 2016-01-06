

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


#include <player/devices/processors/Volume_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_volume.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>
#include <string/key_pattern.h>

#include <stdint.h>
#include <stdlib.h>


static void apply_volume(
        int buf_count,
        const float* in_buffers[buf_count],
        float* out_buffers[buf_count],
        float* scale_buffer,
        float global_scale,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(buf_count > 0);
    assert(in_buffers != NULL);
    assert(out_buffers != NULL);
    assert(global_scale >= 0);
    assert(buf_start >= 0);

    // Copy input to output with global scaling
    for (int ch = 0; ch < buf_count; ++ch)
    {
        const float* in = in_buffers[ch];
        float* out = out_buffers[ch];
        if ((in == NULL) || (out == NULL))
            continue;

        for (int32_t frame = buf_start; frame < buf_stop; ++frame)
            out[frame] = in[frame] * global_scale;
    }

    // Scale output based on control stream
    if (scale_buffer != NULL)
    {
        for (int ch = 0; ch < buf_count; ++ch)
        {
            float* out = out_buffers[ch];
            if (out == NULL)
                continue;

            for (int32_t i = buf_start; i < buf_stop; ++i)
                out[i] *= scale_buffer[i];
        }
    }

    return;
}


typedef struct Volume_pstate
{
    Proc_state parent;
    float scale;
} Volume_pstate;


bool Volume_pstate_set_volume(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);

    Volume_pstate* vol_state = (Volume_pstate*)dstate;

    if (isfinite(value))
        vol_state->scale = dB_to_scale(value);
    else
        vol_state->scale = 1.0;

    return true;
}


static void Volume_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Volume_pstate* vol_pstate = (Volume_pstate*)dstate;

    // Get input
    const float* in_bufs[] =
    {
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 0),
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 1),
    };

    // Get output
    float* out_bufs[] =
    {
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_SEND, 0),
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_SEND, 1),
    };

    // Get control stream
    float* scale_buf =
        Device_state_get_audio_buffer_contents_mut(dstate, DEVICE_PORT_TYPE_RECEIVE, 2);

    apply_volume(
            2, in_bufs, out_bufs, scale_buf, vol_pstate->scale, buf_start, buf_stop);

    return;
}


Device_state* new_Volume_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Volume_pstate* vol_state = memory_alloc_item(Volume_pstate);
    if ((vol_state == NULL) ||
            !Proc_state_init(&vol_state->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(vol_state);
        return NULL;
    }

    vol_state->parent.render_mixed = Volume_pstate_render_mixed;

    vol_state->scale = 1.0;

    return (Device_state*)vol_state;
}


typedef struct Volume_vstate
{
    Voice_state parent;
} Volume_vstate;


size_t Volume_vstate_get_size(void)
{
    return sizeof(Volume_vstate);
}


int32_t Volume_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get input
    const float* in_bufs[] =
    {
        Proc_state_get_voice_buffer_contents_mut(
                proc_state, DEVICE_PORT_TYPE_RECEIVE, 0),
        Proc_state_get_voice_buffer_contents_mut(
                proc_state, DEVICE_PORT_TYPE_RECEIVE, 1),
    };
    if ((in_bufs[0] == NULL) && (in_bufs[1] == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    float* out_bufs[] =
    {
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0),
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 1),
    };

    // Get control stream
    float* scale_buf = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 2);

    const Volume_pstate* vol_pstate = (const Volume_pstate*)proc_state;
    apply_volume(
            2, in_bufs, out_bufs, scale_buf, vol_pstate->scale, buf_start, buf_stop);

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Volume_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Volume_vstate_render_voice;

    return;
}


