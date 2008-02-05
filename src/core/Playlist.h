

/*
 * Copyright 2008 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_PLAYLIST_H
#define K_PLAYLIST_H


#include <Player.h>


typedef struct Playlist
{
	Player* first;
} Playlist;


/**
 * Creates a new Playlist.
 *
 * \return   The new Playlist if successful, or \c NULL if memory allocation
 *           failed.
 */
Playlist* new_Playlist(void);


/**
 * Inserts a Player into the Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param player     The Player -- must not be \c NULL.
 */
void Playlist_ins(Playlist* playlist, Player* player);


/**
 * Gets a Player from the Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param id         The ID of the Player.
 *
 * \return   The Player if found, otherwise \c NULL.
 */
Player* Playlist_get(Playlist* playlist, int32_t id);


/**
 * Removes a Player from the Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param player     The Player -- must not be \c NULL.
 */
void Playlist_remove(Playlist* playlist, Player* player);


/**
 * Destroys an existing Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 */
void del_Playlist(Playlist* playlist);


#endif // K_PLAYLIST_H


