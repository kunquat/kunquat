

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


#ifndef K_PLAYER_H
#define K_PLAYER_H


#include <stdint.h>

#include <Module.h>


typedef struct Player Player;


/**
 * Creates a new Player.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The new Player if successful, or \c NULL if memory allocation
 *           failed.
 */
Player* new_Player(const Module* module);


/**
 * Plays music.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param nframes   The number of frames to be rendered -- must be >= \c 0.
 *                  The actual number of frames rendered may be anything
 *                  between \c 0 and \a nframes.
 */
void Player_play(Player* player, int32_t nframes);


/**
 * Gets the number of frames available in the internal audio buffer.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The number of frames available.
 */
int32_t Player_get_frames_available(Player* player);


/**
 * Destroys the Player.
 *
 * \param player   The Player, or \c NULL.
 */
void del_Player(Player* player);


#endif // K_PLAYER_H


