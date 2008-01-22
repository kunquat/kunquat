

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


#ifndef K_PLAYDATA_H
#define K_PLAYDATA_H


#include <stdint.h>

#include <Order.h>
#include <Channel.h>


/**
 * Playback states.
 */
typedef enum Play_mode
{
	STOP = 0,       ///< Don't play.
	PLAY_PATTERN,   ///< Play one pattern.
	PLAY_SONG,      ///< Play a song.
	PLAY_LAST       ///< Sentinel value -- never used as a mode.
} Play_mode;


#define PAT_CHANNELS (64)


typedef struct Playdata
{
	/// Current playback mode.
	Play_mode play;
	/// Mixing frequency.
	uint32_t freq;
	/// Size of a tick in frames.
	uint16_t tick_size;
	/// The Order lists.
	Order* order;
	/// The global event queue.
	Event_queue* events;
	/// The number of beats played since the start of playback.
	Reltime play_time;
	/// The number of frames mixed since the start of playback.
	uint64_t play_frames;
	/// Current tempo.
	double tempo;
	/// Current subsong -- only relevant if \a play = \c PLAY_SONG.
	uint16_t subsong;
	/// Current order -- only relevant if \a play = \c PLAY_SONG.
	uint16_t order_index;
	/// Current pattern.
	int16_t pattern;
	/// Current position inside a pattern.
	Reltime pos;
	/// The Voice pool used.
	Voice_pool* voice_pool;
	/// The channels used.
	Channel* channels[PAT_CHANNELS];
} Playdata;


/**
 * Creates a new Playdata object.
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param pool   The Voice pool to be used -- must not be \c NULL.
 * \param song   The Song object to which the new Playdata object is assigned
 *               -- must not be \c NULL.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
//Playdata* new_Playdata(Voice_pool* pool, Song* song);


/**
 * Does mixing according to the state of the Playdata object.
 *
 * \param data      The Playdata object -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t Playdata_mix(Playdata* data, uint32_t nframes);


/**
 * Sets the playback mode.
 *
 * \param data   The Playdata object -- must not be \c NULL.
 * \param mode   The playback mode -- must be >= \a STOP and < \a PLAY_LAST.
 */
void Playdata_set_state(Playdata* data, Play_mode mode);


/**
 * Deletes a Playdata object.
 *
 * \param data   The Playdata object -- must not be \c NULL.
 */
void del_Playdata(Playdata* data);


#endif // K_PLAYDATA_H


