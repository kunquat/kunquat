

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_interface.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <memory.h>

#include <stdbool.h>
#include <stdlib.h>


Au_interface* new_Au_interface(void)
{
    Au_interface* iface = memory_alloc_item(Au_interface);
    if (iface == NULL)
        return NULL;

    if (!Device_init(&iface->parent, false))
    {
        del_Au_interface(iface);
        return NULL;
    }

    Device_set_existent(&iface->parent, true);
    Device_set_mixed_signals(&iface->parent, true);

    return iface;
}


void del_Au_interface(Au_interface* iface)
{
    if (iface == NULL)
        return;

    Device_deinit(&iface->parent);
    memory_free(iface);

    return;
}


