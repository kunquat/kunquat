

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


#ifndef K_DSP_TYPE_H
#define K_DSP_TYPE_H


#include <stdint.h>

#include <DSP.h>
#include <DSP_freeverb.h>
#include <DSP_volume.h>


typedef struct DSP_type
{
    char* type;
    DSP* (*cons)(uint32_t buffer_size, uint32_t mix_rate);
} DSP_type;


static const DSP_type DSP_types[] =
{
    { "freeverb", new_DSP_freeverb },
    { "volume", new_DSP_volume },
    { NULL, NULL }
};


#endif // K_DSP_TYPE_H


