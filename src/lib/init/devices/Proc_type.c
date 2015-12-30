

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


#include <init/devices/Proc_type.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_add.h>
#include <init/devices/processors/Proc_chorus.h>
#include <init/devices/processors/Proc_debug.h>
#include <init/devices/processors/Proc_delay.h>
#include <init/devices/processors/Proc_envgen.h>
#include <init/devices/processors/Proc_freeverb.h>
#include <init/devices/processors/Proc_gaincomp.h>
#include <init/devices/processors/Proc_noise.h>
#include <init/devices/processors/Proc_ringmod.h>
#include <init/devices/processors/Proc_sample.h>
#include <init/devices/processors/Proc_volume.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>


struct Proc_type
{
    const char* type;
    Proc_cons* cons;
    Proc_property* property;
};


static const Proc_type proc_types[] =
{
    { "debug", new_Proc_debug, NULL },
    { "add", new_Proc_add, Proc_add_property },
    { "chorus", new_Proc_chorus, NULL },
    { "delay", new_Proc_delay, NULL },
    { "envgen", new_Proc_envgen, Proc_envgen_property },
    { "freeverb", new_Proc_freeverb, NULL },
    { "gaincomp", new_Proc_gaincomp, NULL },
    { "noise", new_Proc_noise, Proc_noise_property },
    { "ringmod", new_Proc_ringmod, NULL },
    { "sample", new_Proc_sample, Proc_sample_property },
    { "volume", new_Proc_volume, Proc_volume_property },
    { NULL, NULL, NULL }
};


Proc_cons* Proc_type_find_cons(const char* type)
{
    assert(type != NULL);

    for (int i = 0; proc_types[i].type != NULL; ++i)
    {
        if (string_eq(type, proc_types[i].type))
            return proc_types[i].cons;
    }

    return NULL;
}


Proc_property* Proc_type_find_property(const char* type)
{
    assert(type != NULL);

    for (int i = 0; proc_types[i].type != NULL; ++i)
    {
        if (string_eq(type, proc_types[i].type))
            return proc_types[i].property;
    }

    assert(false);
    return NULL;
}


