

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

#include <Reltime.h>
#include <Subsong_table.h>
#include <Channel.h>
#include <Voice_pool.h>
#include <Ins_table.h>
#include <kunquat/limits.h>
#include <kunquat/frame.h>


/**
 * Playback states.
 */
typedef enum Play_mode
{
    STOP = 0,       ///< Don't play.
    PLAY_EVENT,     ///< Play a single event.
    PLAY_PATTERN,   ///< Play one pattern.
    PLAY_SUBSONG,   ///< Play one subsong.
    PLAY_SONG,      ///< Play all subsongs.
    PLAY_LAST       ///< Sentinel value -- never used as a mode.
} Play_mode;


typedef struct Playdata
{
    bool silent;                      ///< \c true if this Playdata is used for statistics only.
    Play_mode mode;                   ///< Current playback mode.
    uint32_t freq;                    ///< Mixing frequency.
    uint32_t old_freq;                ///< Old mixing frequency (used to detect freq change).
//  uint16_t tick_size;               ///< Size of a tick in frames. TODO: implement if needed
    Subsong_table* subsongs;          ///< The Subsongs.
    Event_queue* events;              ///< The global event queue.
    Event_queue* ins_events;          ///< The Instrument event queue.
    Reltime play_time;                ///< The number of beats played since the start of playback.
    uint64_t play_frames;             ///< The number of frames mixed since the start of playback.

    int buf_count;                    ///< Number of buffers used for mixing.
    kqt_frame** bufs;                 ///< The (top-level) buffers.
    Scale** scales;                   ///< The Scales.
    Scale** active_scale;             ///< A reference to the currently active Scale.

    double volume;                    ///< Current global volume.
    int volume_slide;                 ///< Global volume slide (0 = no slide, -1 = down, 1 = up).
    double volume_slide_target;       ///< Target volume of the global volume slide.
    double volume_slide_frames;       ///< Number of frames to complete the slide.
    double volume_slide_update;       ///< The update factor of the slide.

    double tempo;                     ///< Current tempo.
    int tempo_slide;                  ///< Tempo slide state (0 = no slide, -1 = down, 1 = up).
    double tempo_slide_target;        ///< Final target tempo of the tempo slide.
    Reltime tempo_slide_left;         ///< The total time left to finish the tempo slide.
    double tempo_slide_int_target;    ///< Intermediate target tempo of the tempo slide.
    Reltime tempo_slide_int_left;     ///< Time left until shifting tempo.
    double tempo_slide_update;        ///< The update amount of the tempo slide.
    double old_tempo;                 ///< Old tempo (used to detect tempo change).

    Reltime delay_left;               ///< The amount of pattern delay left.
    int delay_event_index;            ///< Position of the delay event.

    uint16_t subsong;                 ///< Current subsong -- used when \a play == \c PLAY_SONG.
    uint16_t section;                 ///< Current section -- used when \a play == \c PLAY_SONG.
    int16_t pattern;                  ///< Current pattern.
    Reltime pos;                      ///< Current position inside a pattern.
    Voice_pool* voice_pool;           ///< The Voice pool used.
    Column_iter* citer;               ///< Column iterator.
    Channel* channels[KQT_COLUMNS_MAX]; ///< The channels used.
    uint16_t active_voices;             ///< Number of Voices used simultaneously.
    double min_amps[KQT_BUFFERS_MAX];   ///< Minimum amplitude values encountered.
    double max_amps[KQT_BUFFERS_MAX];   ///< Maximum amplitude values encountered.
    uint64_t clipped[KQT_BUFFERS_MAX];  ///< Number of clipped frames encountered.
} Playdata;


/**
 * Creates a new Playdata object.
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param insts       The Instrument table -- must not be \c NULL.
 * \param buf_count   Number of buffers used for mixing -- must be > \c 0.
 * \param bufs        The mixing buffers -- must not be \c NULL.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
Playdata* new_Playdata(Ins_table* insts,
                       int buf_count,
                       kqt_frame** bufs);


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
 * Sets the subsong in the Playdata.
 *
 * \param play      The Playdata -- must not be \c NULL.
 * \param subsong   The subsong number -- must be >= \c 0 and < \c KQT_SUBSONGS_MAX.
 */
void Playdata_set_subsong(Playdata* play, int subsong);


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


