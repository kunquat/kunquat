

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Volume_state.h>

#include <devices/Device.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Proc_volume.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>
#include <string/key_pattern.h>

#include <stdint.h>
#include <stdlib.h>


typedef struct Volume_pstate
{
    Proc_state parent;
    Linear_controls volume;
} Volume_pstate;


static bool Volume_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Volume_pstate* vol_state = (Volume_pstate*)dstate;
    Linear_controls_set_audio_rate(&vol_state->volume, audio_rate);

    return true;
}


static void Volume_pstate_set_tempo(Device_state* dstate, double tempo)
{
    assert(dstate != NULL);
    assert(tempo > 0);

    Volume_pstate* vol_state = (Volume_pstate*)dstate;
    Linear_controls_set_tempo(&vol_state->volume, tempo);

    return;
}


static void Volume_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Volume_pstate* vol_state = (Volume_pstate*)dstate;
    const Device_impl* dimpl = dstate->device->dimpl;

    Linear_controls_init(&vol_state->volume);

    const double* vol_dB = Device_params_get_float(
            dimpl->device->dparams, "p_f_volume.json");
    if (vol_dB != NULL && isfinite(*vol_dB))
        Linear_controls_set_value(&vol_state->volume, *vol_dB);
    else
        Linear_controls_set_value(&vol_state->volume, 0.0);

    return;
}


bool Volume_pstate_set_volume(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Volume_pstate* vol_state = (Volume_pstate*)dstate;
    Linear_controls_set_value(&vol_state->volume, value);

    return true;
}


void Volume_pstate_render_mixed(
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

    Volume_pstate* vol_state = (Volume_pstate*)dstate;

    // Update real-time control
    static const int CONTROL_WORK_BUFFER_VOLUME = WORK_BUFFER_IMPL_1;

    const Work_buffer* control_wb = Work_buffers_get_buffer(
            wbs, CONTROL_WORK_BUFFER_VOLUME);
    Linear_controls_fill_work_buffer(
            &vol_state->volume, control_wb, buf_start, buf_stop);
    const float* control_values = Work_buffer_get_contents(control_wb);

    // Get port buffers
    float* in_data[] = { NULL, NULL };
    float* out_data[] = { NULL, NULL };
    get_raw_input(&vol_state->parent.parent, 0, in_data);
    get_raw_output(&vol_state->parent.parent, 0, out_data);

    for (int32_t frame = buf_start; frame < buf_stop; ++frame)
    {
        const float scale = dB_to_scale(control_values[frame]);
        out_data[0][frame] += in_data[0][frame] * scale;
        out_data[1][frame] += in_data[1][frame] * scale;
    }

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

    vol_state->parent.set_audio_rate = Volume_pstate_set_audio_rate;
    vol_state->parent.set_tempo = Volume_pstate_set_tempo;
    vol_state->parent.reset = Volume_pstate_reset;
    vol_state->parent.render_mixed = Volume_pstate_render_mixed;

    Linear_controls_init(&vol_state->volume);
    Linear_controls_set_audio_rate(&vol_state->volume, audio_rate);
    Linear_controls_set_value(&vol_state->volume, 0.0);

    return &vol_state->parent.parent;
}


Linear_controls* Volume_pstate_get_cv_controls_volume(
        Device_state* dstate, const Key_indices indices)
{
    assert(dstate != NULL);
    ignore(indices);

    Volume_pstate* vol_state = (Volume_pstate*)dstate;

    return &vol_state->volume;
}


typedef struct Volume_vstate
{
    Voice_state parent;
    Linear_controls volume;
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

    const Processor* proc = (const Processor*)proc_state->parent.device;
    const Proc_volume* vol = (const Proc_volume*)proc_state->parent.device->dimpl;

    Volume_vstate* vol_vstate = (Volume_vstate*)vstate;

    // Update real-time control
    static const int CONTROL_WORK_BUFFER_VOLUME = WORK_BUFFER_IMPL_1;

    const Work_buffer* control_wb =
        Work_buffers_get_buffer(wbs, CONTROL_WORK_BUFFER_VOLUME);
    Linear_controls_set_tempo(&vol_vstate->volume, tempo);
    Linear_controls_fill_work_buffer(
            &vol_vstate->volume, control_wb, buf_start, buf_stop);
    float* control_values = Work_buffer_get_contents_mut(control_wb);

    // Get buffers
    Audio_buffer* in_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (in_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }
    assert(out_buffer != NULL);

    // Convert real-time control values
    for (int32_t i = buf_start; i < buf_stop; ++i)
        control_values[i] = dB_to_scale(control_values[i]);

    // Scale
    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in_values = Audio_buffer_get_buffer(in_buffer, ch);
        float* out_values = Audio_buffer_get_buffer(out_buffer, ch);

        const float scale = vol->scale;

        if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
        {
            const float* actual_forces = Work_buffers_get_buffer_contents(
                    wbs, WORK_BUFFER_ACTUAL_FORCES);

            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] =
                    in_values[i] * scale * actual_forces[i] * control_values[i];
        }
        else
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] = in_values[i] * scale * control_values[i];
        }
    }

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Volume_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Volume_vstate_render_voice;

    Volume_vstate* vol_vstate = (Volume_vstate*)vstate;

    Linear_controls_init(&vol_vstate->volume);
    Linear_controls_set_audio_rate(&vol_vstate->volume, proc_state->parent.audio_rate);
    Linear_controls_set_value(&vol_vstate->volume, 0.0);

    return;
}


Linear_controls* Volume_vstate_get_cv_controls_volume(
        Voice_state* vstate, const Device_state* dstate, const Key_indices indices)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    ignore(indices);

    Volume_vstate* vol_vstate = (Volume_vstate*)vstate;

    return &vol_vstate->volume;
}


