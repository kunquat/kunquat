

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2016
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_noise.h>

#include <debug/assert.h>
#include <init/devices/Device_params.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/devices/processors/Noise_state.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void del_Proc_noise(Device_impl* proc_impl);


Device_impl* new_Proc_noise(void)
{
    Proc_noise* noise = memory_alloc_item(Proc_noise);
    if (noise == NULL)
        return NULL;

    if (!Device_impl_init(&noise->parent, del_Proc_noise))
    {
        del_Device_impl(&noise->parent);
        return NULL;
    }

    noise->parent.create_pstate = new_Noise_pstate;
    noise->parent.get_vstate_size = Noise_vstate_get_size;
    noise->parent.init_vstate = Noise_vstate_init;

    return &noise->parent;
}


static void del_Proc_noise(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_noise* noise = (Proc_noise*)dimpl;
    memory_free(noise);

    return;
}


