

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_gaincomp.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/param_types/Envelope.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Gaincomp_state.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Set_bool_func     Proc_gc_set_map_enabled;
static Set_envelope_func Proc_gc_set_map;

static bool Proc_gaincomp_init(Device_impl* dimpl);

static void del_Proc_gaincomp(Device_impl* dimpl);


Device_impl* new_Proc_gaincomp(Processor* proc)
{
    Proc_gaincomp* gc = memory_alloc_item(Proc_gaincomp);
    if (gc == NULL)
        return NULL;

    gc->parent.device = (Device*)proc;

    Device_impl_register_init(&gc->parent, Proc_gaincomp_init);
    Device_impl_register_destroy(&gc->parent, del_Proc_gaincomp);

    return &gc->parent;
}


static bool Proc_gaincomp_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_gaincomp* gc = (Proc_gaincomp*)dimpl;

    Device_set_state_creator(dimpl->device, new_Gaincomp_pstate);

    Processor* proc = (Processor*)gc->parent.device;
    proc->init_vstate = Gaincomp_vstate_init;

    gc->is_map_enabled = false;
    gc->map = NULL;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val)                   \
    reg_success &= Device_impl_register_set_##type(               \
            &gc->parent, key, def_val, Proc_gc_set_##field, NULL)

    REGISTER_SET(bool,      map_enabled,    "p_b_map_enabled.json",     false);
    REGISTER_SET(envelope,  map,            "p_e_map.json",             NULL);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    return true;
}


static bool Proc_gc_set_map_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_gaincomp* gc = (Proc_gaincomp*)dimpl;
    gc->is_map_enabled = value;

    return true;
}


static bool Proc_gc_set_map(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

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


