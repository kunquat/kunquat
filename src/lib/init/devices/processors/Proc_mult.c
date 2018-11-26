

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_mult.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <memory.h>
#include <player/devices/processors/Mult_state.h>

#include <stdbool.h>
#include <stdlib.h>


static void del_Proc_mult(Device_impl* dimpl);


Device_impl* new_Proc_mult(void)
{
    Proc_mult* mult = memory_alloc_item(Proc_mult);
    if (mult == NULL)
        return NULL;

    if (!Device_impl_init(&mult->parent, del_Proc_mult))
    {
        del_Device_impl(&mult->parent);
        return NULL;
    }

    mult->parent.get_port_groups = Mult_get_port_groups;
    mult->parent.create_pstate = new_Mult_pstate;
    mult->parent.get_vstate_size = Mult_vstate_get_size;
    mult->parent.render_voice = Mult_vstate_render_voice;

    return &mult->parent;
}


static void del_Proc_mult(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_mult* mult = (Proc_mult*)dimpl;
    memory_free(mult);

    return;
}


