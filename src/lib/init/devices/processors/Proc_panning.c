

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_panning.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Panning_state.h>

#include <stdlib.h>


static Set_float_func Proc_panning_set_panning;

static void del_Proc_panning(Device_impl* dimpl);


Device_impl* new_Proc_panning(void)
{
    Proc_panning* panning = memory_alloc_item(Proc_panning);
    if (panning == NULL)
        return NULL;

    panning->panning = 0;

    if (!Device_impl_init(&panning->parent, del_Proc_panning))
    {
        del_Device_impl(&panning->parent);
        return NULL;
    }

    panning->parent.create_pstate = new_Panning_pstate;
    panning->parent.get_vstate_size = Panning_vstate_get_size;
    panning->parent.init_vstate = Panning_vstate_init;

    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &panning->parent,
            "p_f_panning.json",
            0,
            Proc_panning_set_panning,
            Panning_pstate_set_panning);

    reg_success &= Device_impl_create_cv_float(
            &panning->parent,
            "panning",
            Panning_pstate_get_cv_controls_panning,
            Panning_vstate_get_cv_controls_panning);

    if (!reg_success)
    {
        del_Device_impl(&panning->parent);
        return NULL;
    }

    return &panning->parent;
}


static bool Proc_panning_set_panning(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Proc_panning* panning = (Proc_panning*)dimpl;
    panning->panning = value;

    return true;
}


static void del_Proc_panning(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_panning* panning = (Proc_panning*)dimpl;
    memory_free(panning);

    return;
}

