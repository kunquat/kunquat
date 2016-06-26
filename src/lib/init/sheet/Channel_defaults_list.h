

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CHANNEL_DEFAULTS_LIST_H
#define KQT_CHANNEL_DEFAULTS_LIST_H


#include <kunquat/limits.h>
#include <init/sheet/Channel_defaults.h>
#include <string/Streader.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * List of channel defaults of a Song.
 */
typedef struct Channel_defaults_list
{
    Channel_defaults ch_defaults[KQT_CHANNELS_MAX];
} Channel_defaults_list;


/**
 * Create a new Channel defaults list from JSON data.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Channel defaults list if successful, or \c NULL if
 *           an error occurred.
 */
Channel_defaults_list* new_Channel_defaults_list(Streader* sr);


/**
 * Get Channel defaults from the Channel defaults list.
 *
 * \param cdl        The Channel defaults list, or \c NULL.
 * \param ch_index   The channel index -- must be >= \c 0 and
 *                   < \c KQT_CHANNELS_MAX.
 *
 * \return   The Channel defaults.
 */
const Channel_defaults* Channel_defaults_list_get(
        const Channel_defaults_list* cdl, int32_t ch_index);


/**
 * Destroy an existing Channel defaults list.
 *
 * \param cdl   The Channel defaults list, or \c NULL.
 */
void del_Channel_defaults_list(Channel_defaults_list* cdl);


#endif // KQT_CHANNEL_DEFAULTS_LIST_H


