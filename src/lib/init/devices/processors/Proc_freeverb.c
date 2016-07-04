

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_freeverb.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Freeverb_state.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const double initial_reflect = 20;
static const double initial_damp = 20;
static const double initial_wet = 1 / 3.0; // 3.0 = scale_wet
static const double initial_width = 1;


static Set_float_func Proc_freeverb_set_initial_refl;
static Set_float_func Proc_freeverb_set_initial_damp;

static void Proc_freeverb_update_reflectivity(Proc_freeverb* freeverb, double reflect);
static void Proc_freeverb_update_damp(Proc_freeverb* freeverb, double damp);
static void Proc_freeverb_update_gain(Proc_freeverb* freeverb, double gain);
static void Proc_freeverb_update_wet(Proc_freeverb* freeverb, double wet);

static void del_Proc_freeverb(Device_impl* dimpl);


Device_impl* new_Proc_freeverb(void)
{
    Proc_freeverb* freeverb = memory_alloc_item(Proc_freeverb);
    if (freeverb == NULL)
        return NULL;

    if (!Device_impl_init(&freeverb->parent, del_Proc_freeverb))
    {
        del_Device_impl(&freeverb->parent);
        return NULL;
    }

    freeverb->parent.create_pstate = new_Freeverb_pstate;

    // Register key set handlers
    if (!(REGISTER_SET_FIXED_STATE(
                freeverb, float, initial_refl, "p_f_refl.json", initial_reflect) &&
            REGISTER_SET_FIXED_STATE(
                freeverb, float, initial_damp, "p_f_damp.json", initial_damp)
        ))
    {
        del_Device_impl(&freeverb->parent);
        return NULL;
    }

    freeverb->gain = 0;
    freeverb->reflect_setting = 0;
    freeverb->damp_setting = 0;
    freeverb->wet = 0;
    freeverb->wet1 = 0;
    freeverb->wet2 = 0;
    freeverb->width = 0;

    Proc_freeverb_update_reflectivity(freeverb, initial_reflect);
    Proc_freeverb_update_damp(freeverb, initial_damp);
    static const double fixed_gain = 0.015;
    Proc_freeverb_update_gain(freeverb, fixed_gain);
    Proc_freeverb_update_wet(freeverb, initial_wet);

    return &freeverb->parent;
}


static bool Proc_freeverb_set_initial_refl(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;

    if (value > 200)
        value = 200;
    else if (value < 0)
        value = 0;

    Proc_freeverb_update_reflectivity(freeverb, value);

    return true;
}


static bool Proc_freeverb_set_initial_damp(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;

    if (value > 100)
        value = 100;
    else if (value < 0)
        value = 0;

    Proc_freeverb_update_damp(freeverb, value);

    return true;
}


static void Proc_freeverb_update_reflectivity(Proc_freeverb* freeverb, double reflect)
{
    rassert(freeverb != NULL);

    freeverb->reflect_setting = reflect;

    return;
}


static void Proc_freeverb_update_damp(Proc_freeverb* freeverb, double damp)
{
    rassert(freeverb != NULL);

    freeverb->damp_setting = damp;

    return;
}


static void Proc_freeverb_update_gain(Proc_freeverb* freeverb, double gain)
{
    rassert(freeverb != NULL);

    freeverb->gain = gain;

    return;
}


static void Proc_freeverb_update_wet(Proc_freeverb* freeverb, double wet)
{
    rassert(freeverb != NULL);

    static const double scale_wet = 3;
    freeverb->wet = wet * scale_wet;
    freeverb->width = initial_width;

    freeverb->wet1 = freeverb->wet * (freeverb->width * 0.5 + 0.5);
    freeverb->wet2 = freeverb->wet * ((1 - freeverb->width) * 0.5);

    return;
}


static void del_Proc_freeverb(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;
    memory_free(freeverb);

    return;
}


