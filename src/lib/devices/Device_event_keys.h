

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


#ifndef K_DEVICE_EVENT_KEYS_H
#define K_DEVICE_EVENT_KEYS_H


#include <devices/Device_params.h>
#include <player/Player.h>


typedef enum
{
    DEVICE_EVENT_TYPE_GENERATOR = 0,
} Device_event_type;


/**
 * Parse the Device Event list.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param type     The type of Events -- must be \c DEVICE_EVENT_TYPE_GENERATOR
 *                 or \c DEVICE_EVENT_TYPE_DSP.
 * \param player   The Player -- must not be \c NULL.
 * \param str      The textual description.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will _not_ be
 *           modified if memory allocation failed.
 */
#if 0
bool Device_params_parse_events(
        Device_params* params,
        Device_event_type type,
        Player* player,
        char* str,
        Read_state* state);
#endif


#endif // K_DEVICE_EVENT_KEYS


