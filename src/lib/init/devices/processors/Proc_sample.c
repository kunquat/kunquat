

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
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
#include <init/devices/Proc_cons.h>
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

    sample_p->parent.get_vstate_size = Sample_vstate_get_size;
    sample_p->parent.init_vstate = Sample_vstate_init;
    sample_p->parent.render_voice = Sample_vstate_render_voice;

    return &sample_p->parent;
}


void del_Proc_sample(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_sample* sample_p = (Proc_sample*)dimpl;
    memory_free(sample_p);

    return;
}


