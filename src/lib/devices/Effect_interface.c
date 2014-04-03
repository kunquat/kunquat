

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <debug/assert.h>
#include <devices/Device.h>
#include <devices/Effect_interface.h>
#include <memory.h>


Effect_interface* new_Effect_interface()
{
    Effect_interface* ei = memory_alloc_item(Effect_interface);
    if (ei == NULL)
        return NULL;

    if (!Device_init(&ei->parent, false))
    {
        del_Effect_interface(ei);
        return NULL;
    }

    Device_set_existent(&ei->parent, true);

    return ei;
}


void del_Effect_interface(Effect_interface* ei)
{
    if (ei == NULL)
        return;

    Device_deinit(&ei->parent);
    memory_free(ei);

    return;
}


