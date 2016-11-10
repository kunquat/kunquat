

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


static const int CONTROL_WB_PANNING = WORK_BUFFER_IMPL_1;


static void apply_panning(
        const Work_buffers* wbs,
        const float* pan_values,
        float def_pan,
        float* in_buffers[2],
        float* out_buffers[2],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    rassert(isfinite(def_pan));
    rassert(def_pan >= -1);
    rassert(def_pan <= 1);
    rassert(in_buffers != NULL);
    rassert(out_buffers != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);

    float* pannings = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_PANNING);

    if (pan_values != NULL)
    {
        // Get clamped panning input
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            float pan_value = pan_values[i];
            pan_value = clamp(pan_value, -1, 1);
            pannings[i] = pan_value;
        }
    }
    else
    {
        // Get our default panning
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pannings[i] = def_pan;
    }

    // Clamp the input values
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float panning = pannings[i];
        panning = clamp(panning, -1, 1);
        pannings[i] = panning;
    }

    // Apply panning
    // TODO: revisit panning formula
    {
        const float* in_buf = in_buffers[0];
        float* out_buf = out_buffers[0];
        if ((in_buf != NULL) && (out_buf != NULL))
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_buf[i] = in_buf[i] * (1 - pannings[i]);
        }
    }

    {
        const float* in_buf = in_buffers[1];
        float* out_buf = out_buffers[1];
        if ((in_buf != NULL) && (out_buf != NULL))
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_buf[i] = in_buf[i] * (1 + pannings[i]);
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
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Panning_pstate* ppstate = (Panning_pstate*)dstate;

    // Get panning values
    const float* pan_values = Device_thread_state_get_mixed_buffer_contents_mut(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING);

    // Get input
    float* in_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_in_buffers(
            proc_ts, PORT_IN_AUDIO_L, PORT_IN_AUDIO_COUNT, in_buffers);

    // Get output
    float* out_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_buffers);

    apply_panning(
            wbs,
            pan_values,
            (float)ppstate->def_panning,
            in_buffers,
            out_buffers,
            buf_start,
            buf_stop,
            dstate->audio_rate);

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


typedef struct Panning_vstate
{
    Voice_state parent;
    double def_panning;
} Panning_vstate;


int32_t Panning_vstate_get_size(void)
{
    return sizeof(Panning_vstate);
}


static int32_t Panning_vstate_render_voice(
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

    Panning_vstate* pvstate = (Panning_vstate*)vstate;

    // Get panning values
    const float* pan_values = Proc_state_get_voice_buffer_contents_mut(
            proc_state, proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PANNING);

    // Get input
    float* in_buffers[2] = { NULL };
    Proc_state_get_voice_audio_in_buffers(
            proc_state, proc_ts, PORT_IN_AUDIO_L, PORT_IN_AUDIO_COUNT, in_buffers);
    if ((in_buffers[0] == NULL) && (in_buffers[1] == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_state, proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_buffers);

    const Device_state* dstate = (const Device_state*)proc_state;

    apply_panning(
            wbs,
            pan_values,
            (float)pvstate->def_panning,
            in_buffers,
            out_buffers,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    return buf_stop;
}


void Panning_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Panning_vstate_render_voice;

    Panning_vstate* pvstate = (Panning_vstate*)vstate;

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_panning* panning = (const Proc_panning*)dstate->device->dimpl;

    pvstate->def_panning = panning->panning;

    return;
}


