

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


#ifndef K_PLAYER_H
#define K_PLAYER_H


#include <stdint.h>

#include <Playdata.h>
#include <Song.h>
#include <Voice_pool.h>


#define MAX_VOICES (1024)


typedef struct Player
{
    Song* song;
    Playdata* play;
    int32_t id;
    Voice_pool* voices;
    struct Player* prev;
    struct Player* next;
} Player;


/**
 * Creates a new Player.
 *
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param voices   The number of Voices -- must be > \c 0 and < \c MAX_VOICES.
 * \param song     The Song -- must not be \c NULL.
 *
 * \return   The new Player if successful, or \c NULL if memory allocation
 *           failed.
 */
Player* new_Player(uint32_t freq, uint16_t voices, Song* song);


/**
 * Gets the ID of the Player.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The ID.
 */
int32_t Player_get_id(Player* player);


/**
 * Gets the Song of the Player.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The Song.
 */
Song* Player_get_song(Player* player);


/**
 * Gets the Playdata of the Player.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The Playdata.
 */
Playdata* Player_get_playdata(Player* player);


/**
 * Does mixing according to the state of the Player.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t Player_mix(Player* player, uint32_t nframes);


/**
 * Plays one Event. The caller should have set the Event in the desired
 * Channel beforehand.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void Player_play_event(Player* player);


/**
 * Plays one Pattern.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param num      The number of the Pattern -- must be >= \c 0 and
 *                 < \c PATTERNS_MAX.
 * \param tempo    The tempo -- must be > \c 0.
 */
void Player_play_pattern(Player* player, int16_t num, double tempo);


/**
 * Plays a subsong.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param num      The number of the subsong -- must be < \c SUBSONGS_MAX.
 */
void Player_play_subsong(Player* player, uint16_t subsong);


/**
 * Plays the default subsong of the Song.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void Player_play_song(Player* player);


/**
 * Stops playback.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void Player_stop(Player* player);


/**
 * Sets a new mixing frequency.
 * 
 * \param player   The Player -- must not be \c NULL.
 * \param freq     The mixing frequency -- must be > \c 0.
 */
void Player_set_mix_freq(Player* player, uint32_t freq);


/**
 * Destroys an existing Player.
 *
 * The Song inside the player is also destroyed.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void del_Player(Player* player);


#endif // K_PLAYER_H


