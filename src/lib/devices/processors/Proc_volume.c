

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


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Proc_volume.h>
#include <player/Proc_state.h>
#include <string/common.h>
#include <memory.h>


typedef struct Volume_state
{
    Proc_state parent;
    double scale;
} Volume_state;


typedef struct Proc_volume
{
    Device_impl parent;
    double scale;
} Proc_volume;


static Device_state* Proc_volume_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Proc_volume_reset(const Device_impl* dimpl, Device_state* dstate);

static Set_float_func Proc_volume_set_volume;

static Set_state_float_func Proc_volume_set_state_volume;

static Set_cv_float_func Proc_volume_set_cv_volume;


static bool Proc_volume_init(Device_impl* dimpl);

static Proc_process_vstate_func Proc_volume_process_vstate;

static void Proc_volume_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
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

    Device_set_process(volume->parent.device, Proc_volume_process);

    Device_set_state_creator(volume->parent.device, Proc_volume_create_state);

    Device_impl_register_reset_device_state(&volume->parent, Proc_volume_reset);

    Processor* proc = (Processor*)volume->parent.device;
    proc->process_vstate = Proc_volume_process_vstate;

    // Register key and control variable handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &volume->parent,
            "p_f_volume.json",
            0.0,
            Proc_volume_set_volume,
            Proc_volume_set_state_volume);

    reg_success &= Device_impl_register_updaters_cv_float(
            &volume->parent, "v", Proc_volume_set_cv_volume,
            NULL, NULL, NULL, NULL, NULL, NULL);

    if (!reg_success)
        return false;

    volume->scale = 1.0;

    return true;
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

    vol_state->scale = 1.0;

    return &vol_state->parent.parent;
}


static void Proc_volume_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    Volume_state* vol_state = (Volume_state*)dstate;

    const double* vol_dB = Device_params_get_float(
            dimpl->device->dparams, "p_f_volume.json");
    if (vol_dB != NULL && isfinite(*vol_dB))
        vol_state->scale = exp2(*vol_dB / 6);
    else
        vol_state->scale = 1.0;

    return;
}


static double dB_to_scale(double vol_dB)
{
    return isfinite(vol_dB) ? exp2(vol_dB / 6) : 1.0;
}


static bool Proc_volume_set_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_volume* volume = (Proc_volume*)dimpl;
    volume->scale = dB_to_scale(value);

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

    Proc_volume_set_cv_volume(dimpl, dstate, indices, value);

    return true;
}


static void Proc_volume_set_cv_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    Volume_state* vol_state = (Volume_state*)dstate;
    vol_state->scale = dB_to_scale(value);

    return;
}


static uint32_t Proc_volume_process_vstate(
        const Processor* proc,
        Proc_state* proc_state,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    Volume_state* vol_state = (Volume_state*)proc_state;

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

    // Scale
    for (int ch = 0; ch < 2; ++ch)
    {
        const kqt_frame* in_values = Audio_buffer_get_buffer(in_buffer, ch);
        kqt_frame* out_values = Audio_buffer_get_buffer(out_buffer, ch);

        const float scale = vol_state->scale;

        if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
        {
            const float* actual_forces = Work_buffers_get_buffer_contents(
                    wbs, WORK_BUFFER_ACTUAL_FORCES);

            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] = in_values[i] * scale * actual_forces[i];
        }
        else
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] = in_values[i] * scale;
        }
    }

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


static void Proc_volume_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Volume_state* vol_state = (Volume_state*)Device_states_get_state(
            states, Device_get_id(device));
    assert(vol_state != NULL);

    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    get_raw_input(&vol_state->parent.parent, 0, in_data);
    get_raw_output(&vol_state->parent.parent, 0, out_data);

    for (uint32_t frame = start; frame < until; ++frame)
    {
        out_data[0][frame] += in_data[0][frame] * vol_state->scale;
        out_data[1][frame] += in_data[1][frame] * vol_state->scale;
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


