

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


#include <init/devices/processors/Proc_sample.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <init/devices/Device_params.h>
#include <init/devices/param_types/Hit_map.h>
#include <init/devices/param_types/Sample.h>
#include <init/devices/param_types/Wavpack.h>
#include <init/devices/Processor.h>
#include <memory.h>
#include <player/devices/processors/Sample_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool Proc_sample_init(Device_impl* dimpl);

static void del_Proc_sample(Device_impl* dimpl);


Device_impl* new_Proc_sample(Processor* proc)
{
    Proc_sample* sample_p = memory_alloc_item(Proc_sample);
    if (sample_p == NULL)
        return NULL;

    sample_p->parent.device = (Device*)proc;

    Device_impl_register_init(&sample_p->parent, Proc_sample_init);
    Device_impl_register_destroy(&sample_p->parent, del_Proc_sample);

    return &sample_p->parent;
}


static bool Proc_sample_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_sample* sample_p = (Proc_sample*)dimpl;

    Processor* proc = (Processor*)sample_p->parent.device;
    proc->init_vstate = Sample_vstate_init;

    return true;
}


const char* Proc_sample_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", Sample_vstate_get_size());

        return size_str;
    }
    else if (string_eq(property_type, "proc_state_vars"))
    {
        static const char* vars_str = "["
            "[\"I\", \"e\"], " // expression
            "[\"I\", \"s\"]"   // source
            "]";
        return vars_str;
    }

    return NULL;
}


void del_Proc_sample(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_sample* sample_p = (Proc_sample*)dimpl;
    memory_free(sample_p);

    return;
}


