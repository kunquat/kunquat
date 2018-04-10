

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_bitcrusher.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Bitcrusher_state.h>

#include <stdbool.h>
#include <stdlib.h>


static Set_float_func   Proc_bitcrusher_set_cutoff;
static Set_float_func   Proc_bitcrusher_set_resolution;
static Set_float_func   Proc_bitcrusher_set_res_ignore_min;

static void del_Proc_bitcrusher(Device_impl* dimpl);


Device_impl* new_Proc_bitcrusher(void)
{
    Proc_bitcrusher* bitcrusher = memory_alloc_item(Proc_bitcrusher);
    if (bitcrusher == NULL)
        return NULL;

    if (!Device_impl_init(&bitcrusher->parent, del_Proc_bitcrusher))
    {
        del_Device_impl(&bitcrusher->parent);
        return NULL;
    }

    bitcrusher->parent.create_pstate = new_Bitcrusher_pstate;
    bitcrusher->parent.get_vstate_size = Bitcrusher_vstate_get_size;
    bitcrusher->parent.init_vstate = Bitcrusher_vstate_init;
    bitcrusher->parent.render_voice = Bitcrusher_vstate_render_voice;

    bitcrusher->cutoff = BITCRUSHER_DEFAULT_CUTOFF;
    bitcrusher->resolution = BITCRUSHER_DEFAULT_RESOLUTION;
    bitcrusher->res_ignore_min = BITCRUSHER_DEFAULT_RES_IGNORE_MIN;

    if (!(REGISTER_SET_FIXED_STATE(
                bitcrusher,
                float,
                cutoff,
                "p_f_cutoff.json",
                BITCRUSHER_DEFAULT_CUTOFF) &&
            REGISTER_SET_FIXED_STATE(
                bitcrusher,
                float,
                resolution,
                "p_f_resolution.json",
                BITCRUSHER_DEFAULT_RESOLUTION) &&
            REGISTER_SET_FIXED_STATE(
                bitcrusher,
                float,
                res_ignore_min,
                "p_f_res_ignore_min.json",
                BITCRUSHER_DEFAULT_RES_IGNORE_MIN)
         ))
    {
        del_Device_impl(&bitcrusher->parent);
        return NULL;
    }

    return &bitcrusher->parent;
}


static bool Proc_bitcrusher_set_cutoff(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_bitcrusher* bc = (Proc_bitcrusher*)dimpl;
    bc->cutoff = value;

    return true;
}


static bool Proc_bitcrusher_set_resolution(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_bitcrusher* bc = (Proc_bitcrusher*)dimpl;
    bc->resolution = max(1.0, value);

    return true;
}


static bool Proc_bitcrusher_set_res_ignore_min(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_bitcrusher* bc = (Proc_bitcrusher*)dimpl;
    bc->res_ignore_min = max(1.0, value);

    return true;
}


static void del_Proc_bitcrusher(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_bitcrusher* bc = (Proc_bitcrusher*)dimpl;
    memory_free(bc);

    return;
}


