

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_gaincomp.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/param_types/Envelope.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Gaincomp_state.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Set_bool_func     Proc_gc_set_map_enabled;
static Set_envelope_func Proc_gc_set_map;

static void del_Proc_gaincomp(Device_impl* dimpl);


Device_impl* new_Proc_gaincomp(void)
{
    Proc_gaincomp* gc = memory_alloc_item(Proc_gaincomp);
    if (gc == NULL)
        return NULL;

    if (!Device_impl_init(&gc->parent, del_Proc_gaincomp))
    {
        del_Device_impl(&gc->parent);
        return NULL;
    }

    gc->parent.create_pstate = new_Gaincomp_pstate;
    gc->parent.init_vstate = Gaincomp_vstate_init;

    gc->is_map_enabled = false;
    gc->map = NULL;

    if (!(REGISTER_SET_FIXED_STATE(
                gc, bool, map_enabled, "p_b_map_enabled.json", false) &&
            REGISTER_SET_FIXED_STATE(gc, envelope, map, "p_e_map.json", NULL)
        ))
    {
        del_Device_impl(&gc->parent);
        return NULL;
    }

    return &gc->parent;
}


static bool Proc_gc_set_map_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_gaincomp* gc = (Proc_gaincomp*)dimpl;
    gc->is_map_enabled = value;

    return true;
}


static bool Proc_gc_set_map(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_gaincomp* gc = (Proc_gaincomp*)dimpl;

    bool valid = true;
    if (value != NULL && Envelope_node_count(value) > 1)
    {
        double* node = Envelope_get_node(value, 0);
        if (node[0] != 0)
            valid = false;

        node = Envelope_get_node(value, Envelope_node_count(value) - 1);
        if (node[0] != 1)
            valid = false;

        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            node = Envelope_get_node(value, i);
            if (node[1] < 0)
            {
                valid = false;
                break;
            }
        }
    }
    else
    {
        valid = false;
    }

    gc->map = valid ? value : NULL;

    return true;
}


static void del_Proc_gaincomp(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_gaincomp* gc = (Proc_gaincomp*)dimpl;
    memory_free(gc);

    return;
}


