

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


#include <init/devices/processors/Proc_debug.h>

#include <debug/assert.h>
#include <memory.h>
#include <player/devices/processors/Debug_state.h>
#include <string/common.h>

#include <stdlib.h>


static Set_bool_func Proc_debug_set_single_pulse;

static void del_Proc_debug(Device_impl* dimpl);


Device_impl* new_Proc_debug(void)
{
    Proc_debug* debug = memory_alloc_item(Proc_debug);
    if (debug == NULL)
        return NULL;

    if (!Device_impl_init(&debug->parent, del_Proc_debug))
    {
        del_Device_impl(&debug->parent);
        return NULL;
    }

    debug->parent.init_vstate = Debug_vstate_init;

    if (!Device_impl_register_set_bool(
                &debug->parent,
                "p_b_single_pulse.json",
                false,
                Proc_debug_set_single_pulse,
                NULL))
    {
        del_Device_impl(&debug->parent);
        return NULL;
    }

    debug->single_pulse = false;

    return &debug->parent;
}


static bool Proc_debug_set_single_pulse(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_debug* debug = (Proc_debug*)dimpl;
    debug->single_pulse = value;

    return true;
}


static void del_Proc_debug(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_debug* debug = (Proc_debug*)dimpl;
    memory_free(debug);

    return;
}


