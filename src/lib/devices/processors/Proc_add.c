

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_add.h>

#include <debug/assert.h>
#include <devices/Processor.h>
#include <devices/param_types/Sample.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Add_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool Proc_add_init(Device_impl* dimpl);

static double sine(double phase, double modifier);

static Set_sample_func  Proc_add_set_base;
static Set_bool_func    Proc_add_set_ramp_attack;
static Set_float_func   Proc_add_set_tone_pitch;
static Set_float_func   Proc_add_set_tone_volume;
static Set_float_func   Proc_add_set_tone_panning;

static void del_Proc_add(Device_impl* dimpl);


Device_impl* new_Proc_add(Processor* proc)
{
    Proc_add* add = memory_alloc_item(Proc_add);
    if (add == NULL)
        return NULL;

    add->parent.device = (Device*)proc;

    Device_impl_register_init(&add->parent, Proc_add_init);
    Device_impl_register_destroy(&add->parent, del_Proc_add);

    return &add->parent;
}


static bool Proc_add_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    Processor* proc = (Processor*)add->parent.device;
    proc->init_vstate = Add_vstate_init;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val) \
    reg_success &= Device_impl_register_set_##type(                       \
            &add->parent, key, def_val, Proc_add_set_##field, NULL)

    REGISTER_SET(sample,    base,           "p_base.wav",               NULL);
    REGISTER_SET(bool,      ramp_attack,    "p_b_ramp_attack.json",     true);
    REGISTER_SET(float,     tone_pitch,     "tone_XX/p_f_pitch.json",   NAN);
    REGISTER_SET(float,     tone_volume,    "tone_XX/p_f_volume.json",  NAN);
    REGISTER_SET(float,     tone_panning,   "tone_XX/p_f_pan.json",     0.0);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    add->base = NULL;
    add->is_ramp_attack_enabled = true;

    float* buf = memory_alloc_items(float, ADD_BASE_FUNC_SIZE + 1);
    if (buf == NULL)
    {
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->base = new_Sample_from_buffers(&buf, 1, ADD_BASE_FUNC_SIZE + 1);
    if (add->base == NULL)
    {
        memory_free(buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    for (int i = 0; i < ADD_BASE_FUNC_SIZE; ++i)
        buf[i] = sine((double)i / ADD_BASE_FUNC_SIZE, 0);
    buf[ADD_BASE_FUNC_SIZE] = buf[0];

    for (int h = 0; h < ADD_TONES_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
        add->tones[h].panning = 0;
    }

    add->tones[0].pitch_factor = 1.0;
    add->tones[0].volume_factor = 1.0;

    return true;
}


const char* Proc_add_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", Add_vstate_get_size());

        return size_str;
    }

    return NULL;
}


static double sine(double phase, double modifier)
{
    ignore(modifier);
    return -sin(phase * PI * 2);
}


static void fill_buf(float* buf, const Sample* sample)
{
    assert(buf != NULL);

    if ((sample != NULL) && (sample->data[0] != NULL) && sample->is_float)
    {
        int32_t available = min(sample->len, ADD_BASE_FUNC_SIZE);

        const float* from_buf = sample->data[0];

        for (int i = 0; i < available; ++i)
            buf[i] = clamp(from_buf[i], -1.0, 1.0);
        for (int i = available; i < ADD_BASE_FUNC_SIZE; ++i)
            buf[i] = 0;
    }
    else
    {
        for (int i = 0; i < ADD_BASE_FUNC_SIZE; ++i)
            buf[i] = sine((double)i / ADD_BASE_FUNC_SIZE, 0);
    }

    buf[ADD_BASE_FUNC_SIZE] = buf[0];

    return;
}


static bool Proc_add_set_base(
        Device_impl* dimpl, const Key_indices indices, const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    fill_buf(Sample_get_buffer(add->base, 0), value);

    return true;
}


static bool Proc_add_set_ramp_attack(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->is_ramp_attack_enabled = enabled;

    return true;
}


static bool Proc_add_set_tone_pitch(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->tones[ti].pitch_factor = value;
    else
        add->tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_volume(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (isfinite(value))
        add->tones[ti].volume_factor = exp2(value / 6.0);
    else
        add->tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_panning(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value >= -1.0 && value <= 1.0)
        add->tones[ti].panning = value;
    else
        add->tones[ti].panning = 0.0;

    return true;
}


static void del_Proc_add(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_add* add = (Proc_add*)dimpl;
    del_Sample(add->base);
    memory_free(add);

    return;
}


