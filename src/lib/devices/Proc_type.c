

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


#include <stdlib.h>

#include <debug/assert.h>
#include <devices/Proc_type.h>
#include <devices/processors/Proc_debug.h>
#include <devices/processors/Proc_add.h>
#include <devices/processors/Proc_chorus.h>
#include <devices/processors/Proc_delay.h>
#include <devices/processors/Proc_freeverb.h>
#include <devices/processors/Proc_gc.h>
#include <devices/processors/Proc_noise.h>
#include <devices/processors/Proc_pcm.h>
#include <devices/processors/Proc_volume.h>
#include <string/common.h>


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
    { "freeverb", new_Proc_freeverb, NULL },
    { "gaincomp", new_Proc_gc, NULL },
    { "noise", new_Proc_noise, Proc_noise_property },
    { "pcm", new_Proc_pcm, Proc_pcm_property },
    { "volume", new_Proc_volume, NULL },
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


