

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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
#include <Generator_noise.h>
#include <Generator_pcm.h>
#include <Generator_pulse.h>
#include <Generator_sawtooth.h>
#include <Generator_sine.h>
#include <Generator_square303.h>
#include <Generator_triangle.h>
#include <string_common.h>
#include <xassert.h>


const Gen_type gen_types[] =
{
    { "debug", new_Generator_debug, NULL },
    { "noise", new_Generator_noise, Generator_noise_property },
    { "pcm", new_Generator_pcm, Generator_pcm_property },
    { "pulse", new_Generator_pulse, Generator_pulse_property },
    { "sawtooth", new_Generator_sawtooth, Generator_sawtooth_property },
    { "sine", new_Generator_sine, Generator_sine_property },
    { "square303", new_Generator_square303, Generator_square303_property },
    { "triangle", new_Generator_triangle, Generator_triangle_property },
    { NULL, NULL, NULL }
};


Generator_cons* Gen_type_find_cons(char* type)
{
    assert(type != NULL);
    for (int i = 0; gen_types[i].type != NULL; ++i)
    {
        if (string_eq(type, gen_types[i].type))
        {
            return gen_types[i].cons;
        }
    }
    return NULL;
}


Generator_property* Gen_type_find_property(char* type)
{
    assert(type != NULL);
    for (int i = 0; gen_types[i].type != NULL; ++i)
    {
        if (string_eq(type, gen_types[i].type))
        {
            return gen_types[i].property;
        }
    }
    assert(false);
    return NULL;
}


