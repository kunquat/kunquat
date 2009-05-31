

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


#ifndef K_SONG_H
#define K_SONG_H


#include <stdint.h>

#include <Song_limits.h>
#include <frame_t.h>
#include <Order.h>
#include <Pat_table.h>
#include <Ins_table.h>
#include <Note_table.h>
#include <Playdata.h>


typedef struct Song
{
    int buf_count;                      ///< Number of buffers used for mixing.
    uint32_t buf_size;                  ///< Buffer size.
    frame_t** bufs;                     ///< Buffers.
    frame_t* priv_bufs[BUF_COUNT_MAX];  ///< Private buffers.
    frame_t* voice_bufs[BUF_COUNT_MAX]; ///< Temporary buffers for Voices.
    Order* order;                       ///< The Order lists.
    Pat_table* pats;                    ///< The Patterns.
    Ins_table* insts;                   ///< The Instruments.
    Note_table* notes[NOTE_TABLES_MAX]; ///< The Note tables.
    Note_table** active_notes;          ///< A reference to the currently active Note table.
    Event_queue* events;                ///< Global events.
    wchar_t name[SONG_NAME_MAX];        ///< The name of the Song.
    double mix_vol;                     ///< Mixing volume.
    uint16_t init_subsong;              ///< Initial subsong number.
} Song;


/**
 * Creates a new Song.
 * The caller shall eventually call del_Song() to destroy the Song returned.
 *
 * \param buf_count   Number of buffers to allocate -- must be >= \c 1 and
 *                    <= \a BUF_COUNT_MAX. Typically, this is 2 (stereo).
 * \param buf_size    Size of a buffer -- must be > \c 0.
 * \param events      The maximum number of global events per tick -- must be
 *                    > \c 0.
 *
 * \see del_Song()
 *
 * \return   The new Song if successful, or \c NULL if memory allocation
 *           failed.
 */
Song* new_Song(int buf_count, uint32_t buf_size, uint8_t events);


/**
 * Mixes a portion of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param nframes   The amount of frames to be mixed.
 * \param play      The Playdata containing the playback state -- must not be
 *                  \c NULL.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t Song_mix(Song* song, uint32_t nframes, Playdata* play);


/**
 * Sets the name of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param name   The name to be set -- must not be \c NULL. Only the first
 *               SONG_NAME_MAX - 1 characters will be used.
 */
void Song_set_name(Song* song, wchar_t* name);


/**
 * Gets the name of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The name.
 */
wchar_t* Song_get_name(Song* song);


/**
 * Sets the initial tempo of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param subsong   The subsong index -- must be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 * \param tempo     The tempo -- must be finite and > \c 0.
 */
void Song_set_tempo(Song* song, int subsong, double tempo);


/**
 * Gets the initial tempo of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param subsong   The subsong index -- must be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 *
 * \return   The tempo.
 */
double Song_get_tempo(Song* song, int subsong);


/**
 * Sets the mixing volume of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param mix_vol   The volume -- must be finite or -INFINITY.
 */
void Song_set_mix_vol(Song* song, double mix_vol);


/**
 * Gets the mixing volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The mixing volume.
 */
double Song_get_mix_vol(Song* song);


/**
 * Sets the initial global volume of the Song.
 *
 * \param song         The Song -- must not be \c NULL.
 * \param subsong      The subsong index -- must be >= \c 0 and
 *                     < \c SUBSONGS_MAX.
 * \param global_vol   The volume -- must be finite or -INFINITY.
 */
void Song_set_global_vol(Song* song, int subsong, double global_vol);


/**
 * Gets the initial global volume of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param subsong   The subsong index -- must be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 *
 * \return   The global volume.
 */
double Song_get_global_vol(Song* song, int subsong);


/**
 * Sets the initial subsong of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param num    The subsong number -- must be < \c SUBSONGS_MAX.
 */
void Song_set_subsong(Song* song, uint16_t num);


/**
 * Gets the initial subsong of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The initial subsong number.
 */
uint16_t Song_get_subsong(Song* song);


/**
 * Sets the number of buffers in the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param count   The number of buffers -- must be > \c 0 and
 *                < \c BUF_COUNT_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_set_buf_count(Song* song, int count);


/**
 * Gets the number of buffers in the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The number of buffers.
 */
int Song_get_buf_count(Song* song);


/**
 * Sets the size of buffers in the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param size   The new size for a single buffer -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_set_buf_size(Song* song, uint32_t size);


/**
 * Gets the size of a single buffer in the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The buffer size.
 */
uint32_t Song_get_buf_size(Song* song);


/**
 * Gets the buffers from the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The buffers.
 */
frame_t** Song_get_bufs(Song* song);


/**
 * Gets the Voice buffers from the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Voice buffers.
 */
frame_t** Song_get_voice_bufs(Song* song);


/**
 * Gets the Order lists from the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Order list.
 */
Order* Song_get_order(Song* song);


/**
 * Gets the Patterns of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Pattern table.
 */
Pat_table* Song_get_pats(Song* song);


/**
 * Gets the Instruments of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Instrument table.
 */
Ins_table* Song_get_insts(Song* song);


/**
 * Gets a Note table of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Note table index -- must be >= 0 and < NOTE_TABLES_MAX.
 *
 * \return   The Note table.
 */
Note_table* Song_get_notes(Song* song, int index);


/**
 * Gets an indirect reference to the active Note table of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 *
 * \return   The reference.
 */
Note_table** Song_get_active_notes(Song* song);


/**
 * Creates a new Note table for the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Note table index -- must be >= 0 and < NOTE_TABLES_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_create_notes(Song* song, int index);


/**
 * Removes a Note table from the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Note table index -- must be >= 0 and < NOTE_TABLES_MAX.
 *                If the Note table doesn't exist, nothing will be done.
 */
void Song_remove_notes(Song* song, int index);


/**
 * Gets the global Event queue of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Event queue.
 */
Event_queue* Song_get_events(Song* song);


/**
 * Destroys an existing Song.
 *
 * \param song   The Song -- must not be \c NULL.
 */
void del_Song(Song* song);


#endif // K_SONG_H


