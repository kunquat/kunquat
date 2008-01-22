

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


#ifndef K_SONG_H
#define K_SONG_H


#include <stdint.h>

#include <frame_t.h>
#include <Order.h>
#include <Pat_table.h>
#include <Ins_table.h>
#include <Note_table.h>
#include <Playdata.h>


#define SONG_NAME_MAX (128)

#define BUF_COUNT_MAX (2)


typedef struct Song
{
	/// Number of buffers (channels) used for mixing.
	int buf_count;
	/// Buffers.
	frame_t* buf[BUF_COUNT_MAX];
	/// The Order lists.
	Order* order;
	/// The Patterns.
	Pat_table* pats;
	/// The Instruments.
	Ins_table* insts;
	/// The Note table.
	Note_table* notes;
	/// Global events.
	Event_queue* events;
	/// The name of the Song.
	wchar_t name[SONG_NAME_MAX];
	/// Initial tempo.
	double tempo;
	/// Mixing volume.
	double mix_vol;
	/// Initial global volume.
	double global_vol;
} Song;


/**
 * Creates a new Song object.
 * The caller shall eventually call del_Song() to destroy the Song returned.
 *
 * \param buf_count   Number of buffers to allocate -- must be >= \c 1 and
 *                    <= \a BUF_COUNT_MAX. Typically, this is 2 (stereo).
 * \param buf_size    Size of a buffer -- must be > \c 0.
 *
 * \see del_Song()
 *
 * \return   The new Song object if successful, or \c NULL if memory
 *           allocation failed.
 */
Song* new_Song(int buf_count, uint32_t buf_size);


/**
 * Mixes a portion of the Song object.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param nframes   The amount of frames to be mixed.
 * \param play      The Playdata containing the playback state.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t Song_mix(Song* song, uint32_t nframes, Playdata* play);


/**
 * Gets a buffer from the Song object.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param buf    The buffer number -- must be < \a BUF_COUNT_MAX.
 *
 * \return   The buffer if one exists, otherwise \c NULL.
 */
frame_t* Song_get_buf(Song* song, uint8_t buf);


/**
 * Destroys an existing Song object.
 *
 * \param song   The Song -- must not be \c NULL.
 */
void del_Song(Song* song);


#endif // K_SONG_H


