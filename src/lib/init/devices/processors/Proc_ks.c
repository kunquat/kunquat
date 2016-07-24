

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


#include <init/devices/processors/Proc_ks.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Ks_state.h>

#include <stdlib.h>


static Set_float_func Proc_ks_set_damp;

static void del_Proc_ks(Device_impl* dimpl);


Device_impl* new_Proc_ks(void)
{
    Proc_ks* ks = memory_alloc_item(Proc_ks);
    if (ks == NULL)
        return NULL;

    ks->damp = 0;

    if (!Device_impl_init(&ks->parent, del_Proc_ks))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

    if (!REGISTER_SET_FIXED_STATE(ks, float, damp, "p_f_damp.json", 0.0))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

    ks->parent.get_vstate_size = Ks_vstate_get_size;
    ks->parent.init_vstate = Ks_vstate_init;

    return &ks->parent;
}


static bool Proc_ks_set_damp(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;

    if (value >= 0 && value <= 100)
        ks->damp = value;
    else
        ks->damp = 0;

    return true;
}


static void del_Proc_ks(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ks* ks = (Proc_ks*)dimpl;
    memory_free(ks);

    return;
}


