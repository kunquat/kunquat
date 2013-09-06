

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <Device_impl.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_volume.h>
#include <string_common.h>
#include <memory.h>
#include <xassert.h>


typedef struct Volume_state
{
    DSP_state parent;
    double scale;
} Volume_state;


typedef struct DSP_volume
{
    Device_impl parent;
    double scale;
} DSP_volume;


static Device_state* DSP_volume_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void DSP_volume_reset(const Device_impl* dimpl, Device_state* dstate);

static bool DSP_volume_set_volume(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_volume_update_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);


static void DSP_volume_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


static void del_DSP_volume(Device_impl* dsp_impl);


Device_impl* new_DSP_volume(DSP* dsp, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    DSP_volume* volume = memory_alloc_item(DSP_volume);
    if (volume == NULL)
        return NULL;

    if (!Device_impl_init(&volume->parent, del_DSP_volume, mix_rate, buffer_size))
    {
        memory_free(volume);
        return NULL;
    }

    volume->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_volume_process);
#if 0
    if (!DSP_init(&volume->parent, del_DSP_volume,
                  DSP_volume_process, buffer_size, mix_rate))
    {
        memory_free(volume);
        return NULL;
    }
#endif

    Device_set_state_creator(volume->parent.device, DSP_volume_create_state);

    Device_impl_register_reset_device_state(&volume->parent, DSP_volume_reset);

    // Register key set/update handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &volume->parent, "p_volume.jsonf", 0.0, DSP_volume_set_volume);

    reg_success &= Device_impl_register_update_state_float(
            &volume->parent, "p_volume.jsonf", DSP_volume_update_state_volume);
    reg_success &= Device_impl_register_update_state_float(
            &volume->parent, "v", DSP_volume_update_state_volume);

    if (!reg_success)
    {
        del_DSP_volume(&volume->parent);
        return NULL;
    }

    Device_register_port(volume->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(volume->parent.device, DEVICE_PORT_TYPE_SEND, 0);

    volume->scale = 1.0;

    return &volume->parent;
}


static Device_state* DSP_volume_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Volume_state* vol_state = memory_alloc_item(Volume_state);
    if (vol_state == NULL)
        return NULL;

    DSP_state_init(&vol_state->parent, device, audio_rate, audio_buffer_size);
    vol_state->scale = 1.0;

    return &vol_state->parent.parent;
}


static void DSP_volume_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    Volume_state* vol_state = (Volume_state*)dstate;

    const double* vol_dB = Device_params_get_float(
            dimpl->device->dparams, "p_volume.jsonf");
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


static bool DSP_volume_set_volume(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_volume* volume = (DSP_volume*)dimpl;
    volume->scale = dB_to_scale(value);

    return true;
}


static bool DSP_volume_update_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    (void)indices;

    Volume_state* vol_state = (Volume_state*)dstate;
    vol_state->scale = dB_to_scale(value);

    return true;
}


static void DSP_volume_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Volume_state* vol_state = (Volume_state*)Device_states_get_state(
            states, Device_get_id(device));
    assert(vol_state != NULL);

    (void)freq;
    (void)tempo;
    //assert(string_eq(volume->parent.type, "volume"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(&vol_state->parent.parent, 0, in_data);
    DSP_get_raw_output(&vol_state->parent.parent, 0, out_data);

    for (uint32_t frame = start; frame < until; ++frame)
    {
        out_data[0][frame] += in_data[0][frame] * vol_state->scale;
        out_data[1][frame] += in_data[1][frame] * vol_state->scale;
    }

    return;
}


static void del_DSP_volume(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "volume"));
    DSP_volume* volume = (DSP_volume*)dsp_impl;
    memory_free(volume);

    return;
}


