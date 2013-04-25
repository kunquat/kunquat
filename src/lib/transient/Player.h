

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
 * Destroys the Player.
 *
 * \param player   The Player, or \c NULL.
 */
void del_Player(Player* player);


#endif // K_PLAYER_H


