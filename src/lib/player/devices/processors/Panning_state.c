

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


#include <player/devices/processors/Panning_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_panning.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdlib.h>


void Panning_get_port_groups(
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


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_PANNING,
    PORT_IN_COUNT,

    PORT_IN_AUDIO_COUNT = PORT_IN_AUDIO_R + 1,
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static void apply_panning(
        const Work_buffer* pan_wb,
        float def_pan,
        float* in_buffer,
        float* out_buffer,
        int32_t frame_count)
{
    rassert(isfinite(def_pan));
    rassert(def_pan >= -1);
    rassert(def_pan <= 1);
    rassert(out_buffer != NULL);
    rassert(frame_count > 0);

    float* in = in_buffer;
    float* out = out_buffer;

    if (in_buffer == NULL)
    {
        const int32_t item_count = frame_count * 2;
        for (int32_t i = 0; i < item_count; ++i)
            *out++ = 0;

        return;
    }

    int32_t pan_const_start = 0;
    if ((pan_wb != NULL) && Work_buffer_is_valid(pan_wb, 0))
        pan_const_start = Work_buffer_get_const_start(pan_wb, 0);

    if (pan_const_start > 0)
    {
        const float* pan = Work_buffer_get_contents(pan_wb, 0);

        const int32_t var_stop = min(pan_const_start, frame_count);

        for (int32_t i = 0; i < var_stop; ++i)
        {
            float pan_value = *pan++;
            pan_value = clamp(pan_value, -1, 1);

            *out++ = *in++ * (1 - pan_value);
            *out++ = *in++ * (1 + pan_value);
        }
    }

    if (pan_const_start < frame_count)
    {
        float fixed_pan = def_pan;
        if ((pan_wb != NULL) && Work_buffer_is_valid(pan_wb, 0))
            fixed_pan = Work_buffer_get_contents(pan_wb, 0)[pan_const_start];

        const float pans[2] = { 1 - fixed_pan, 1 + fixed_pan };

        for (int32_t i = pan_const_start; i < frame_count; ++i)
        {
            *out++ = *in++ * pans[0];
            *out++ = *in++ * pans[1];
        }
    }

    return;
}


typedef struct Panning_pstate
{
    Proc_state parent;
    double def_panning;
} Panning_pstate;


static void Panning_pstate_render_mixed(
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

    Panning_pstate* ppstate = (Panning_pstate*)dstate;

    // Get panning values
    const Work_buffer* pan_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING, NULL);

    // Get input
    float* in_buffer = NULL;
    {
        Work_buffer* in_wb =
            Proc_get_mixed_input_2ch(proc_ts, PORT_IN_AUDIO_L, frame_count);
        if (in_wb != NULL)
            in_buffer = Work_buffer_get_contents_mut(in_wb, 0);
    }

    // Get output
    float* out_buffer = NULL;
    {
        Work_buffer* out_wb = Proc_get_mixed_output_2ch(proc_ts, PORT_OUT_AUDIO_L);
        rassert(out_wb != NULL);
        out_buffer = Work_buffer_get_contents_mut(out_wb, 0);
    }

    apply_panning(
            pan_wb, (float)ppstate->def_panning, in_buffer, out_buffer, frame_count);

    return;
}


bool Panning_pstate_set_panning(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Panning_pstate* ppstate = (Panning_pstate*)dstate;
    ppstate->def_panning = value;

    return true;
}


Device_state* new_Panning_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Panning_pstate* ppstate = memory_alloc_item(Panning_pstate);
    if ((ppstate == NULL) ||
            !Proc_state_init(&ppstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(ppstate);
        return NULL;
    }

    ppstate->parent.render_mixed = Panning_pstate_render_mixed;

    return &ppstate->parent.parent;
}


int32_t Panning_vstate_get_size(void)
{
    return 0;
}


int32_t Panning_vstate_render_voice(
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

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_panning* panning = (const Proc_panning*)dstate->device->dimpl;

    // Get panning values
    const Work_buffer* pan_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING, NULL);

    // Get input
    float* in_buffer = NULL;
    {
        Work_buffer* in_wb =
            Proc_get_voice_input_2ch(proc_ts, PORT_IN_AUDIO_L, frame_count);
        if (in_wb != NULL)
            in_buffer = Work_buffer_get_contents_mut(in_wb, 0);
    }
    if (in_buffer == NULL)
        return 0;

    // Get output
    float* out_buffer = NULL;
    {
        Work_buffer* out_wb = Proc_get_voice_output_2ch(proc_ts, PORT_OUT_AUDIO_L);
        rassert(out_wb != NULL);
        out_buffer = Work_buffer_get_contents_mut(out_wb, 0);
    }

    apply_panning(pan_wb, (float)panning->panning, in_buffer, out_buffer, frame_count);

    return frame_count;
}


