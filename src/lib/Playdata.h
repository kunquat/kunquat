

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


#ifndef K_PLAYDATA_H
#define K_PLAYDATA_H


#include <stdint.h>

#include <kqt_Reltime.h>
#include <Order.h>
#include <Channel.h>
#include <Voice_pool.h>
#include <Ins_table.h>
#include <Song_limits.h>


/**
 * Playback states.
 */
typedef enum Play_mode
{
    STOP = 0,       ///< Don't play.
    PLAY_EVENT,     ///< Play a single event.
    PLAY_PATTERN,   ///< Play one pattern.
    PLAY_SONG,      ///< Play a song.
    PLAY_SILENT,    ///< Don't actually play -- just get some playback statistics.
    PLAY_LAST       ///< Sentinel value -- never used as a mode.
} Play_mode;


typedef struct Playdata
{
    Play_mode mode;                   ///< Current playback mode.
    uint32_t freq;                    ///< Mixing frequency.
//  uint16_t tick_size;               ///< Size of a tick in frames. TODO: implement if needed
    Order* order;                     ///< The Order lists.
    Event_queue* events;              ///< The global event queue.
    kqt_Reltime play_time;            ///< The number of beats played since the start of playback.
    uint64_t play_frames;             ///< The number of frames mixed since the start of playback.
    double tempo;                     ///< Current tempo.
    uint16_t subsong;                 ///< Current subsong -- used when \a play == \c PLAY_SONG.
    uint16_t order_index;             ///< Current order -- used when \a play == \c PLAY_SONG.
    int16_t pattern;                  ///< Current pattern.
    kqt_Reltime pos;                  ///< Current position inside a pattern.
    Voice_pool* voice_pool;           ///< The Voice pool used.
    Column_iter* citer;               ///< Column iterator.
    Channel* channels[COLUMNS_MAX];   ///< The channels used.
    uint16_t active_voices;           ///< Number of Voices used simultaneously.
    double min_amps[BUF_COUNT_MAX];   ///< Minimum amplitude values encountered.
    double max_amps[BUF_COUNT_MAX];   ///< Maximum amplitude values encountered.
    uint64_t clipped[BUF_COUNT_MAX];  ///< Number of clipped frames encountered.
} Playdata;


/**
 * Creates a new Playdata object.
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param pool    The Voice pool to be used -- must not be \c NULL.
 * \param insts   The Instrument table -- must not be \c NULL.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
Playdata* new_Playdata(uint32_t freq, Voice_pool* pool, Ins_table* insts);


/**
 * Creates a new silent Playdata object (used for retrieving statistics).
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param freq    The mixing frequency -- must be > \c 0.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
Playdata* new_Playdata_silent(uint32_t freq);


/**
 * Sets a new mixing frequency.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 * \param freq   The mixing frequency -- must be > \c 0.
 */
void Playdata_set_mix_freq(Playdata* play, uint32_t freq);


/**
 * Resets playback statistics.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 */
void Playdata_reset_stats(Playdata* play);


/**
 * Deletes a Playdata object.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 */
void del_Playdata(Playdata* play);


#endif // K_PLAYDATA_H


