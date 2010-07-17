

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
#include <stdbool.h>
#include <string.h>

#include <Voice_params.h>
#include <xassert.h>


Voice_params* Voice_params_init(Voice_params* params)
{
    assert(params != NULL);
    params->channel_mute = false;
    return params;
}


Voice_params* Voice_params_copy(Voice_params* dest, Voice_params* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(Voice_params));
    return dest;
}


