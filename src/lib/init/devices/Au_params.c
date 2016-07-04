

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_params.h>

#include <debug/assert.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Au_params* Au_params_init(Au_params* aup, uint32_t device_id)
{
    rassert(aup != NULL);
    rassert(device_id > 0);

    aup->device_id = device_id;

    return aup;
}


void Au_params_deinit(Au_params* aup)
{
    if (aup == NULL)
        return;

    return;
}


