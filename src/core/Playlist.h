

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#include <stdbool.h>

#include <Player.h>


typedef struct Playlist
{
    uint32_t buf_count;
    uint32_t buf_size;
    Player* first;
    double max_values[BUF_COUNT_MAX];
    double min_values[BUF_COUNT_MAX];
    bool reset;
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
 * The Player is deleted by the function.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param player     The Player -- must not be \c NULL.
 */
void Playlist_remove(Playlist* playlist, Player* player);


/**
 * Sets the number of buffers.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param count      The buffer count -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Playlist_set_buf_count(Playlist* playlist, int count);


/**
 * Gets the number of buffers.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 *
 * \return   The number of buffers.
 */
int Playlist_get_buf_count(Playlist* playlist);


/**
 * Sets the size of buffers.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param buf_size   The new size of a single buffer -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Playlist_set_buf_size(Playlist* playlist, uint32_t size);


/**
 * Gets the size of a single buffer in the Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 *
 * \return   The buffer size.
 */
uint32_t Playlist_get_buf_size(Playlist* playlist);


/**
 * Sets a new mixing frequency.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param freq       The mixing frequency -- must be > \c 0.
 */
void Playlist_set_mix_freq(Playlist* playlist, uint32_t freq);


/**
 * Mixes the Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param nframes    The number of frames to be mixed.
 * \param bufs       An array of mixing buffers -- must not be \c NULL.
 *                   The array must contain enough mixing buffers (maximum
 *                   amount returned by Song_set_buf_count for each Song).
 *                   The function does not initialise the buffers to 0.
 */
void Playlist_mix(Playlist* playlist, uint32_t nframes, frame_t** bufs);


/**
 * Resets playback statistics.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 */
void Playlist_reset_stats(Playlist* playlist);


/**
 * Schedules a reset of playback statistics.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 */
void Playlist_schedule_reset(Playlist* playlist);


/**
 * Destroys an existing Playlist.
 *
 * \param playlist   The Playlist -- must not be \c NULL. All the Players
 *                   contained in the Playlist are also destroyed.
 */
void del_Playlist(Playlist* playlist);


#endif // K_PLAYLIST_H


