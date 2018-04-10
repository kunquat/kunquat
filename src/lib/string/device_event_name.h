

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


#ifndef KQT_DEVICE_EVENT_NAME
#define KQT_DEVICE_EVENT_NAME


#include <stdbool.h>
#include <stdlib.h>


/**
 * Check if a string is a valid device event name.
 *
 * \param str   The string -- must not be \c NULL.
 *
 * \return   \c true if \a str is a valid device event name, otherwise \c false.
 */
bool is_valid_device_event_name(const char* str);


#endif // KQT_DEVICE_EVENT_NAME


