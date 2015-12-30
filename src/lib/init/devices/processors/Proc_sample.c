

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

#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <memory.h>
#include <player/devices/processors/Sample_state.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void del_Proc_sample(Device_impl* dimpl);


Device_impl* new_Proc_sample(void)
{
    Proc_sample* sample_p = memory_alloc_item(Proc_sample);
    if (sample_p == NULL)
        return NULL;

    if (!Device_impl_init(&sample_p->parent, del_Proc_sample))
    {
        del_Device_impl(&sample_p->parent);
        return NULL;
    }

    sample_p->parent.init_vstate = Sample_vstate_init;

    return &sample_p->parent;
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


