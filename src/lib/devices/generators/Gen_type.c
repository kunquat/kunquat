

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
#include <devices/generators/Gen_type.h>
#include <devices/generators/Generator_debug.h>
#include <devices/generators/Generator_add.h>
#include <devices/generators/Generator_chorus.h>
#include <devices/generators/Generator_delay.h>
#include <devices/generators/Generator_freeverb.h>
#include <devices/generators/Generator_gc.h>
#include <devices/generators/Generator_noise.h>
#include <devices/generators/Generator_pcm.h>
#include <devices/generators/Generator_pulse.h>
#include <devices/generators/Generator_volume.h>
#include <string/common.h>


struct Gen_type
{
    const char* type;
    Generator_cons* cons;
    Generator_property* property;
};


static const Gen_type gen_types[] =
{
    { "debug", new_Generator_debug, NULL },
    { "add", new_Generator_add, Generator_add_property },
    { "chorus", new_Generator_chorus, NULL },
    { "delay", new_Generator_delay, NULL },
    { "freeverb", new_Generator_freeverb, NULL },
    { "gaincomp", new_Generator_gc, NULL },
    { "noise", new_Generator_noise, Generator_noise_property },
    { "pcm", new_Generator_pcm, Generator_pcm_property },
    { "pulse", new_Generator_pulse, Generator_pulse_property },
    { "volume", new_Generator_volume, NULL },
    { NULL, NULL, NULL }
};


Generator_cons* Gen_type_find_cons(const char* type)
{
    assert(type != NULL);

    for (int i = 0; gen_types[i].type != NULL; ++i)
    {
        if (string_eq(type, gen_types[i].type))
            return gen_types[i].cons;
    }

    return NULL;
}


Generator_property* Gen_type_find_property(const char* type)
{
    assert(type != NULL);

    for (int i = 0; gen_types[i].type != NULL; ++i)
    {
        if (string_eq(type, gen_types[i].type))
            return gen_types[i].property;
    }

    assert(false);
    return NULL;
}


