

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
#include <init/devices/Device_params.h>
#include <memory.h>
#include <player/devices/processors/Debug_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static bool Proc_debug_init(Device_impl* dimpl);

static Set_bool_func Proc_debug_set_single_pulse;

static void del_Proc_debug(Device_impl* dimpl);


Device_impl* new_Proc_debug(Processor* proc)
{
    Proc_debug* debug = memory_alloc_item(Proc_debug);
    if (debug == NULL)
        return NULL;

    debug->parent.device = (Device*)proc;

    Device_impl_register_init(&debug->parent, Proc_debug_init);
    Device_impl_register_destroy(&debug->parent, del_Proc_debug);

    return &debug->parent;
}


static bool Proc_debug_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_debug* debug = (Proc_debug*)dimpl;

    Processor* proc = (Processor*)debug->parent.device;
    proc->init_vstate = Debug_vstate_init;

    if (!Device_impl_register_set_bool(
                &debug->parent,
                "p_b_single_pulse.json",
                false,
                Proc_debug_set_single_pulse,
                NULL))
        return false;

    debug->single_pulse = false;

    return true;
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


