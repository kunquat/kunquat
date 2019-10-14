

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_phaser.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Phaser_state.h>

#include <stdlib.h>


static Set_int_func     Proc_phaser_set_stage_count;
static Set_float_func   Proc_phaser_set_cutoff;
static Set_float_func   Proc_phaser_set_notch_separation;
static Set_float_func   Proc_phaser_set_dry_wet_ratio;

static Device_impl_destroy_func del_Proc_phaser;


Device_impl* new_Proc_phaser(void)
{
    Proc_phaser* phaser = memory_alloc_item(Proc_phaser);
    if (phaser == NULL)
        return NULL;

    if (!Device_impl_init(&phaser->parent, del_Proc_phaser))
    {
        del_Device_impl(&phaser->parent);
        return NULL;
    }

    phaser->parent.create_pstate = new_Phaser_pstate;

    phaser->stage_count = PHASER_STAGES_DEFAULT;
    phaser->cutoff = 100;
    phaser->notch_separation = 2; // TODO
    phaser->dry_wet_ratio = 0.5;

    if (!(REGISTER_SET_FIXED_STATE(
                    phaser,
                    int,
                    stage_count,
                    "p_i_stages.json",
                    PHASER_STAGES_DEFAULT) &&
                REGISTER_SET_FIXED_STATE(
                    phaser,
                    float,
                    cutoff,
                    "p_f_cutoff.json",
                    100.0) &&
                REGISTER_SET_FIXED_STATE(
                    phaser,
                    float,
                    notch_separation,
                    "p_f_notch_separation.json",
                    2.0) &&
                REGISTER_SET_FIXED_STATE(
                    phaser,
                    float,
                    dry_wet_ratio,
                    "p_f_dry_wet_ratio.json",
                    0.5)
         ))
    {
        del_Device_impl(&phaser->parent);
        return NULL;
    }

    return &phaser->parent;
}


static bool Proc_phaser_set_stage_count(
        Device_impl* dimpl, const Key_indices indices, int64_t value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_phaser* phaser = (Proc_phaser*)dimpl;
    if ((value < PHASER_STAGES_MIN) || (value > PHASER_STAGES_MAX))
        phaser->stage_count = PHASER_STAGES_DEFAULT;
    else
        phaser->stage_count = (int)value;

    return true;
}


static bool Proc_phaser_set_cutoff(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_phaser* phaser = (Proc_phaser*)dimpl;
    phaser->cutoff = value;

    return true;
}


static bool Proc_phaser_set_notch_separation(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_phaser* phaser = (Proc_phaser*)dimpl;
    if ((value < PHASER_NOTCH_SEP_MIN) || (value > PHASER_NOTCH_SEP_MAX))
        phaser->notch_separation = 2;
    else
        phaser->notch_separation = value;

    return true;
}


static bool Proc_phaser_set_dry_wet_ratio(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_phaser* phaser = (Proc_phaser*)dimpl;
    if ((value < 0) || (value > 1))
        phaser->dry_wet_ratio = 0.5;
    else
        phaser->dry_wet_ratio = value;

    return true;
}


void del_Proc_phaser(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_phaser* phaser = (Proc_phaser*)dimpl;
    memory_free(phaser);

    return;
}


