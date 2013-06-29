

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MASTER_PARAMS_H
#define K_MASTER_PARAMS_H


#include <stdbool.h>
#include <stdint.h>

#include <Bind.h>
#include <Environment.h>
#include <General_state.h>
#include <transient/Position.h>


typedef enum
{
    PLAYBACK_STOPPED = 0,
    PLAYBACK_PATTERN,
    PLAYBACK_SONG,
    PLAYBACK_MODULE,
    PLAYBACK_COUNT
} Playback_state;


typedef struct Master_params
{
    General_state parent;

    Playback_state playback_state;
    bool is_paused;
    Position cur_pos;

    Bind* bind;

    int16_t active_voices;
} Master_params;


/**
 * Initialises the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 * \param env      The Environment -- must not be \c NULL.
 *
 * \return   The parameter \a params if successful, or \c NULL if memory
 *           allocation failed.
 */
Master_params* Master_params_init(Master_params* params, Environment* env);


/**
 * Resets the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 */
void Master_params_reset(Master_params* params);


/**
 * Deinitialises the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 */
void Master_params_deinit(Master_params* params);


#endif // K_MASTER_PARAMS_H


