

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
#include <player/Linear_controls.h>

#include <stdint.h>
#include <stdlib.h>


typedef struct Panning_pstate
{
    Proc_state parent;
    Linear_controls panning;
} Panning_pstate;


static bool Panning_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Panning_pstate* ppstate = (Panning_pstate*)dstate;
    Linear_controls_set_audio_rate(&ppstate->panning, audio_rate);

    return true;
}


static void Panning_pstate_set_tempo(Device_state* dstate, double tempo)
{
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Panning_pstate* ppstate = (Panning_pstate*)dstate;
    Linear_controls_set_tempo(&ppstate->panning, tempo);

    return;
}


static void Panning_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    const Proc_panning* panning = (const Proc_panning*)dstate->device->dimpl;

    Panning_pstate* ppstate = (Panning_pstate*)dstate;
    Linear_controls_set_value(&ppstate->panning, panning->panning);

    return;
}


static const int CONTROL_WB_PANNING = WORK_BUFFER_IMPL_1;


static void apply_panning(
        const Work_buffers* wbs,
        Audio_buffer* in_buffer,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    assert(wbs != NULL);
    assert(in_buffer != NULL);
    assert(out_buffer != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);

    float* pannings = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_PANNING);

    // Clamp the input values
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float panning = pannings[i];
        panning = clamp(panning, -1, 1);
        pannings[i] = panning;
    }

    const float* in_bufs[] =
    {
        Audio_buffer_get_buffer(in_buffer, 0),
        Audio_buffer_get_buffer(in_buffer, 1),
    };

    float* out_bufs[] =
    {
        Audio_buffer_get_buffer(out_buffer, 0),
        Audio_buffer_get_buffer(out_buffer, 1),
    };

    // Apply panning
    // TODO: revisit panning formula
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float panning = pannings[i];
        out_bufs[0][i] = in_bufs[0][i] * (1 - panning);
        out_bufs[1][i] = in_bufs[1][i] * (1 + panning);
    }

    return;
}


static void Panning_pstate_render_mixed(
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

    Panning_pstate* ppstate = (Panning_pstate*)dstate;

    Linear_controls_set_tempo(&ppstate->panning, tempo);

    const Work_buffer* panning_wb = Work_buffers_get_buffer(wbs, CONTROL_WB_PANNING);
    Linear_controls_fill_work_buffer(&ppstate->panning, panning_wb, buf_start, buf_stop);

    // Get input
    Audio_buffer* in_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (in_buffer == NULL)
        return;

    // Get output
    Audio_buffer* out_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    apply_panning(wbs, in_buffer, out_buffer, buf_start, buf_stop, dstate->audio_rate);

    return;
}


bool Panning_pstate_set_panning(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Panning_pstate* ppstate = (Panning_pstate*)dstate;
    Linear_controls_set_value(&ppstate->panning, value);

    return true;
}


Device_state* new_Panning_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Panning_pstate* ppstate = memory_alloc_item(Panning_pstate);
    if ((ppstate == NULL) ||
            !Proc_state_init(&ppstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(ppstate);
        return NULL;
    }

    ppstate->parent.set_audio_rate = Panning_pstate_set_audio_rate;
    ppstate->parent.set_tempo = Panning_pstate_set_tempo;
    ppstate->parent.reset = Panning_pstate_reset;
    ppstate->parent.render_mixed = Panning_pstate_render_mixed;

    Device_state* dstate = (Device_state*)ppstate;
    Linear_controls_init(&ppstate->panning);
    Panning_pstate_set_audio_rate(dstate, audio_rate);

    return dstate;
}


