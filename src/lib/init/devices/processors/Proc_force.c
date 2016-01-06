

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


#include <init/devices/processors/Proc_force.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Force_state.h>

#include <stdlib.h>


static void del_Proc_force(Device_impl* dimpl);


Device_impl* new_Proc_force(void)
{
    Proc_force* force = memory_alloc_item(Proc_force);
    if (force == NULL)
        return NULL;

    if (!Device_impl_init(&force->parent, del_Proc_force))
    {
        del_Device_impl(&force->parent);
        return NULL;
    }

    force->parent.get_vstate_size = Force_vstate_get_size;
    force->parent.init_vstate = Force_vstate_init;

    return &force->parent;
}


static void del_Proc_force(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_force* force = (Proc_force*)dimpl;
    memory_free(force);

    return;
}


