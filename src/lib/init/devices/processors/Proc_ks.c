

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

#include <init/devices/Proc_cons.h>
#include <memory.h>
#include <player/devices/processors/Ks_state.h>

#include <stdlib.h>


static void del_Proc_ks(Device_impl* dimpl);


Device_impl* new_Proc_ks(void)
{
    Proc_ks* ks = memory_alloc_item(Proc_ks);
    if (ks == NULL)
        return NULL;

    if (!Device_impl_init(&ks->parent, del_Proc_ks))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

    ks->parent.get_vstate_size = Ks_vstate_get_size;
    ks->parent.init_vstate = Ks_vstate_init;

    return &ks->parent;
}


static void del_Proc_ks(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ks* ks = (Proc_ks*)dimpl;
    memory_free(ks);

    return;
}


