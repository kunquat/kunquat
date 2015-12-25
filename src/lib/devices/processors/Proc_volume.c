

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_volume.h>

#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Voice_state_volume.h>
#include <mathnum/conversions.h>
#include <player/Linear_controls.h>
#include <player/devices/Proc_state.h>
#include <string/common.h>
#include <memory.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Volume_state
{
    Proc_state parent;
    Linear_controls volume;
} Volume_state;


typedef struct Proc_volume
{
    Device_impl parent;
    double scale;
} Proc_volume;


static Device_state* Proc_volume_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static bool Proc_state_volume_set_audio_rate(Device_state* dstate, int32_t audio_rate);

static void Proc_state_volume_set_tempo(Device_state* dstate, double tempo);

static void Proc_state_volume_reset(Device_state* dstate);

static Set_float_func Proc_volume_set_volume;

static Set_state_float_func Proc_volume_set_state_volume;

static Get_cv_float_controls_mut_func Proc_volume_get_cv_controls_volume;
static Get_voice_cv_float_controls_mut_func Proc_volume_get_voice_cv_controls_volume;


static bool Proc_volume_init(Device_impl* dimpl);

static void Proc_volume_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static Voice_state_render_voice_func Proc_state_volume_render_voice;

static void Proc_state_volume_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);

static void del_Proc_volume(Device_impl* dimpl);


Device_impl* new_Proc_volume(Processor* proc)
{
    Proc_volume* volume = memory_alloc_item(Proc_volume);
    if (volume == NULL)
        return NULL;

    volume->parent.device = (Device*)proc;

    Device_impl_register_init(&volume->parent, Proc_volume_init);
    Device_impl_register_destroy(&volume->parent, del_Proc_volume);

    return &volume->parent;
}


static bool Proc_volume_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_volume* volume = (Proc_volume*)dimpl;

    Device_set_state_creator(volume->parent.device, Proc_volume_create_state);

    Processor* proc = (Processor*)volume->parent.device;
    proc->init_vstate = Proc_volume_init_vstate;

    // Register key and control variable handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &volume->parent,
            "p_f_volume.json",
            0.0,
            Proc_volume_set_volume,
            Proc_volume_set_state_volume);

    if (!reg_success)
        return false;

    Device_impl_cv_float_callbacks* vol_cbs =
        Device_impl_create_cv_float(&volume->parent, "volume");
    if (vol_cbs == NULL)
        return false;
    vol_cbs->get_controls = Proc_volume_get_cv_controls_volume;
    vol_cbs->get_voice_controls = Proc_volume_get_voice_cv_controls_volume;

    volume->scale = 1.0;

    return true;
}


const char* Proc_volume_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_volume));

        return size_str;
    }

    return NULL;
}


static void Proc_volume_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(vstate != NULL);

    vstate->render_voice = Proc_state_volume_render_voice;

    Voice_state_volume* vol_vstate = (Voice_state_volume*)vstate;

    Linear_controls_init(&vol_vstate->volume);
    Linear_controls_set_audio_rate(&vol_vstate->volume, proc_state->parent.audio_rate);
    Linear_controls_set_value(&vol_vstate->volume, 0.0);

    return;
}


static Device_state* Proc_volume_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Volume_state* vol_state = memory_alloc_item(Volume_state);
    if (vol_state == NULL)
        return NULL;

    if (!Proc_state_init(&vol_state->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(vol_state);
        return NULL;
    }

    vol_state->parent.set_audio_rate = Proc_state_volume_set_audio_rate;
    vol_state->parent.set_tempo = Proc_state_volume_set_tempo;
    vol_state->parent.reset = Proc_state_volume_reset;
    vol_state->parent.render_mixed = Proc_state_volume_render_mixed;

    Linear_controls_init(&vol_state->volume);
    Linear_controls_set_audio_rate(&vol_state->volume, audio_rate);
    Linear_controls_set_value(&vol_state->volume, 0.0);

    return &vol_state->parent.parent;
}


static bool Proc_state_volume_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Volume_state* vol_state = (Volume_state*)dstate;

    Linear_controls_set_audio_rate(&vol_state->volume, audio_rate);

    return true;
}


static void Proc_state_volume_set_tempo(Device_state* dstate, double tempo)
{
    assert(dstate != NULL);
    assert(tempo > 0);

    Volume_state* vol_state = (Volume_state*)dstate;

    Linear_controls_set_tempo(&vol_state->volume, tempo);

    return;
}


static void Proc_state_volume_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Volume_state* vol_state = (Volume_state*)dstate;
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


static bool Proc_volume_set_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_volume* volume = (Proc_volume*)dimpl;
    volume->scale = isfinite(value) ? dB_to_scale(value) : 1.0;

    return true;
}


static bool Proc_volume_set_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Volume_state* vol_state = (Volume_state*)dstate;
    Linear_controls_set_value(&vol_state->volume, value);

    return true;
}


static Linear_controls* Proc_volume_get_cv_controls_volume(
        const Device_impl* dimpl, Device_state* dstate, const Key_indices indices)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    ignore(indices);

    Volume_state* vol_state = (Volume_state*)dstate;

    return &vol_state->volume;
}


static Linear_controls* Proc_volume_get_voice_cv_controls_volume(
        const Device_impl* dimpl,
        const Device_state* dstate,
        Voice_state* vstate,
        const Key_indices indices)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(vstate != NULL);
    ignore(indices);

    Voice_state_volume* vol_vstate = (Voice_state_volume*)vstate;

    return &vol_vstate->volume;
}


static int32_t Proc_state_volume_render_voice(
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

    Voice_state_volume* vol_vstate = (Voice_state_volume*)vstate;

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


static void Proc_state_volume_render_mixed(
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

    Volume_state* vol_state = (Volume_state*)dstate;

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


static void del_Proc_volume(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_volume* volume = (Proc_volume*)dimpl;
    memory_free(volume);

    return;
}


