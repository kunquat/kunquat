

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


#include <init/devices/processors/Proc_volume.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>
#include <mathnum/conversions.h>
#include <player/devices/processors/Volume_state.h>
#include <string/common.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Set_float_func Proc_volume_set_volume;

static void del_Proc_volume(Device_impl* dimpl);


Device_impl* new_Proc_volume(void)
{
    Proc_volume* volume = memory_alloc_item(Proc_volume);
    if (volume == NULL)
        return NULL;

    if (!Device_impl_init(&volume->parent, del_Proc_volume))
    {
        del_Device_impl(&volume->parent);
        return NULL;
    }

    volume->parent.create_pstate = new_Volume_pstate;
    volume->parent.get_vstate_size = Volume_vstate_get_size;
    volume->parent.init_vstate = Volume_vstate_init;

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
    {
        del_Device_impl(&volume->parent);
        return NULL;
    }

    volume->scale = 1.0;

    return &volume->parent;
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


