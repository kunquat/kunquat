

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Gen_type.h>
#include <Generator_debug.h>
#include <Generator_add.h>
#include <Generator_noise.h>
#include <Generator_pcm.h>
#include <Generator_pulse.h>
#include <string_common.h>
#include <xassert.h>


struct Gen_type
{
    char* type;
    Generator_cons* cons;
    Generator_property* property;
};


const Gen_type gen_types[] =
{
    { "debug", new_Generator_debug, NULL },
    { "add", new_Generator_add, Generator_add_property },
    { "noise", new_Generator_noise, Generator_noise_property },
    { "pcm", new_Generator_pcm, Generator_pcm_property },
    { "pulse", new_Generator_pulse, Generator_pulse_property },
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


