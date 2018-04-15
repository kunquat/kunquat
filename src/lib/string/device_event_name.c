

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string/device_event_name.h>

#include <debug/assert.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


bool is_valid_device_event_name(const char* str)
{
    rassert(str != NULL);

    const size_t length = strlen(str);
    return (0 < length) && (length <= KQT_DEVICE_EVENT_NAME_MAX) &&
        (strspn(str, KQT_DEVICE_EVENT_CHARS) == length);
}


