

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2015
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_noise.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_params.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Noise_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static bool Proc_noise_init(Device_impl* dimpl);

static void del_Proc_noise(Device_impl* proc_impl);


Device_impl* new_Proc_noise(Processor* proc)
{
    Proc_noise* noise = memory_alloc_item(Proc_noise);
    if (noise == NULL)
        return NULL;

    noise->parent.device = (Device*)proc;

    Device_impl_register_init(&noise->parent, Proc_noise_init);
    Device_impl_register_destroy(&noise->parent, del_Proc_noise);

    return &noise->parent;
}


static bool Proc_noise_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_noise* noise = (Proc_noise*)dimpl;

    Processor* proc = (Processor*)noise->parent.device;
    proc->init_vstate = Noise_vstate_init;

    Device_set_state_creator(noise->parent.device, new_Noise_pstate);

    return true;
}


const char* Proc_noise_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", Noise_vstate_get_size());
        return size_str;
    }
    else if (string_eq(property_type, "proc_state_vars"))
    {
        static const char* vars_str = "[[\"I\", \"o\"]]"; // noise order
        return vars_str;
    }

    return NULL;
}


static void del_Proc_noise(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_noise* noise = (Proc_noise*)dimpl;
    memory_free(noise);

    return;
}


