

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
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
        Work_buffer* pan_wb,
        float def_pan,
        const Work_buffer* in_wbs[2],
        Work_buffer* out_wbs[2],
        int32_t frame_count)
{
    rassert(isfinite(def_pan));
    rassert(def_pan >= -1);
    rassert(def_pan <= 1);
    rassert(in_wbs != NULL);
    rassert(out_wbs != NULL);
    rassert(frame_count > 0);

    int32_t pan_const_start = 0;
    if (Work_buffer_is_valid(pan_wb))
        pan_const_start = Work_buffer_get_const_start(pan_wb);

    if (pan_const_start > 0)
    {
        float* pan = Work_buffer_get_contents_mut(pan_wb);

        const int32_t var_stop = min(pan_const_start, frame_count);

        for (int32_t i = 0; i < var_stop; ++i)
        {
            float pan_value = pan[i];
            pan_value = clamp(pan_value, -1, 1);
            pan[i] = pan_value;
        }

        const float pan_mult[2] = { -1.0f, 1.0f };

        for (int ch = 0; ch < 2; ++ch)
        {
            if (!Work_buffer_is_valid(in_wbs[ch]) || (out_wbs[ch] == NULL))
                continue;

            const float* in = Work_buffer_get_contents(in_wbs[ch]);
            float* out = Work_buffer_get_contents_mut(out_wbs[ch]);

            for (int32_t i = 0; i < var_stop; ++i)
                *out++ = *in++ * (1 + (pan_mult[ch] * pan[i]));
        }
    }

    if (pan_const_start < frame_count)
    {
        float fixed_pan = def_pan;
        if (Work_buffer_is_valid(pan_wb))
            fixed_pan = Work_buffer_get_contents(pan_wb)[pan_const_start];
        fixed_pan = clamp(fixed_pan, -1, 1);

        const float pans[2] = { 1 - fixed_pan, 1 + fixed_pan };

        for (int ch = 0; ch < 2; ++ch)
        {
            if (!Work_buffer_is_valid(in_wbs[ch]) || (out_wbs[ch] == NULL))
                continue;

            const float* in = Work_buffer_get_contents(in_wbs[ch]) + pan_const_start;
            float* out = Work_buffer_get_contents_mut(out_wbs[ch]) + pan_const_start;
            const float pan = pans[ch];

            for (int32_t i = pan_const_start; i < frame_count; ++i)
                *out++ = *in++ * pan;
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
    Work_buffer* pan_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING);

    // Get input
    const Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        in_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);

    if (!Work_buffer_is_valid(in_wbs[0]) && !Work_buffer_is_valid(in_wbs[1]))
        return;

    // Get output
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);

    apply_panning(pan_wb, (float)ppstate->def_panning, in_wbs, out_wbs, frame_count);

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
    Work_buffer* pan_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING);

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

    apply_panning(pan_wb, (float)panning->panning, in_wbs, out_wbs, frame_count);

    return frame_count;
}


