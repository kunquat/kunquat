

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


#ifndef K_GEN_TYPE_H
#define K_GEN_TYPE_H


#include <stdint.h>

#include <Generator.h>
#include <Generator_debug.h>
#include <Generator_noise.h>
#include <Generator_pcm.h>
#include <Generator_pulse.h>
#include <Generator_sawtooth.h>
#include <Generator_sine.h>
#include <Generator_square303.h>
#include <Generator_triangle.h>


typedef struct Gen_type
{
    char* type;
    Generator* (*cons)(uint32_t buffer_size, uint32_t mix_rate);
    char* (*property)(const char* property_type);
} Gen_type;


const Gen_type gen_types[] =
{
    { "debug", new_Generator_debug, NULL },
    { "noise", new_Generator_noise, NULL },
    { "pcm", new_Generator_pcm, NULL },
    { "pulse", new_Generator_pulse, NULL },
    { "sawtooth", new_Generator_sawtooth, NULL },
    { "sine", new_Generator_sine, NULL },
    { "square303", new_Generator_square303, NULL },
    { "triangle", new_Generator_triangle, NULL },
    { NULL, NULL, NULL }
};


#endif // K_GEN_TYPE_H


