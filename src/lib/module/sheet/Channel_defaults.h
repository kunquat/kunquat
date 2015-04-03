

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CHANNEL_DEFAULTS_H
#define K_CHANNEL_DEFAULTS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <string/Streader.h>


/**
 * This structure contains default values that are applied to a Channel
 * on every reset.
 */
typedef struct Channel_defaults
{
    int16_t control_id;
} Channel_defaults;


#define CHANNEL_DEFAULTS_AUTO &(Channel_defaults){ .control_id = 0 }


/**
 * Initialise the Channel defaults with default values.
 *
 * \param chd   The Channel defaults -- must not be \c NULL.
 *
 * \return   The parameter \a chd.
 */
Channel_defaults* Channel_defaults_init(Channel_defaults* chd);


/**
 * Read channel defaults from a JSON string.
 *
 * \param chd   The Channel defaults -- must not be \c NULL.
 * \param sr    The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if parse error occurred.
 */
bool Channel_defaults_read(Channel_defaults* chd, Streader* sr);


#endif // K_CHANNEL_DEFAULTS_H


