

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_ringmod.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Ringmod_state.h>
#include <player/devices/Voice_state.h>

#include <math.h>
#include <stdlib.h>


static bool Proc_ringmod_init(Device_impl* dimpl);

static void del_Proc_ringmod(Device_impl* dimpl);


Device_impl* new_Proc_ringmod(Processor* proc)
{
    Proc_ringmod* ringmod = memory_alloc_item(Proc_ringmod);
    if (ringmod == NULL)
        return NULL;

    ringmod->parent.device = (Device*)proc;

    Device_impl_register_init(&ringmod->parent, Proc_ringmod_init);
    Device_impl_register_destroy(&ringmod->parent, del_Proc_ringmod);

    return &ringmod->parent;
}


static bool Proc_ringmod_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;

    Device_set_state_creator(ringmod->parent.device, new_Ringmod_pstate);

    Processor* proc = (Processor*)ringmod->parent.device;
    proc->init_vstate = Ringmod_vstate_init;

    return true;
}


static void del_Proc_ringmod(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;
    memory_free(ringmod);

    return;
}


