

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
#include <mathnum/conversions.h>
#include <player/Linear_controls.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Volume_states.h>
#include <string/common.h>
#include <memory.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Set_float_func Proc_volume_set_volume;

static bool Proc_volume_init(Device_impl* dimpl);

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

    Device_set_state_creator(volume->parent.device, new_Volume_pstate);

    Processor* proc = (Processor*)volume->parent.device;
    proc->init_vstate = Volume_vstate_init;

    // Register key and control variable handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &volume->parent,
            "p_f_volume.json",
            0.0,
            Proc_volume_set_volume,
            Volume_pstate_set_volume);

    reg_success &= Device_impl_create_cv_float(
            &volume->parent,
            "volume",
            Volume_pstate_get_cv_controls_volume,
            Volume_vstate_get_cv_controls_volume);

    if (!reg_success)
        return false;

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
            snprintf(size_str, 8, "%zd", Volume_vstate_get_size());

        return size_str;
    }

    return NULL;
}


static bool Proc_volume_set_volume(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_volume* volume = (Proc_volume*)dimpl;
    volume->scale = isfinite(value) ? dB_to_scale(value) : 1.0;

    return true;
}


static void del_Proc_volume(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_volume* volume = (Proc_volume*)dimpl;
    memory_free(volume);

    return;
}


