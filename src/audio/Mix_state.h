

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef MIX_STATE_H
#define MIX_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <kunquat/Handle.h>


typedef struct Mix_state
{
    bool playing;            ///< Whether anything is playing.
    uint64_t frames;         ///< Number of frames mixed.
    long long nanoseconds;   ///< Nanoseconds mixed.
    int subsong;             ///< Number of the current Subsong.
    int section;             ///< The current section.
    uint16_t pattern;        ///< The current Pattern index.
    long long beat;          ///< The current beat inside a Pattern.
    long beat_rem;           ///< The remainder of the beat.
    double tempo;            ///< The current tempo (BPM).
    uint16_t voices;         ///< The maximum number of simultaneous Voices since the last update.
    double min_amps[2];      ///< Minimum amplitude values since the last update.
    double max_amps[2];      ///< Maximum amplitude values since the last update.
    uint64_t clipped[2];     ///< Number of clipped frames encountered.
} Mix_state;


/**
 * A new instance of an uninitialised Mix state object with automatic storage
 * allocation.
 * Useful for passing as a parameter to an initialiser.
 */
#define MIX_STATE_AUTO (&(Mix_state){ .playing = false })


/**
 * Initialises the Mix state.
 *
 * \param state   The Mix state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Mix_state* Mix_state_init(Mix_state* state);


/**
 * Copies a Mix state into another.
 *
 * \param dest   The destination Mix state -- must not be \c NULL.
 * \param src    The source Mix state -- must not be \c NULL.
 *
 * \return   The parameter \ə dest.
 */
Mix_state* Mix_state_copy(Mix_state* dest, Mix_state* src);


/**
 * Gets playback statistics from a Kunquat Handle.
 *
 * This is a convenience function that retrieves the information provided by
 * the following functions:
 *
 *    kqt_Handle_end_reached
 *    kqt_Handle_get_position
 *    kqt_Handle_get_position_ns
 *    kqt_Handle_get_tempo
 *    kqt_Handle_get_voice_count
 *    kqt_Handle_get_min_amplitude
 *    kqt_Handle_get_max_amplitude
 *    kqt_Handle_get_clipped
 *    
 * It also calls kqt_Handle_reset_stats.
 *
 * \param mix_state   The Mix state where the statistics shall be written
 *                    -- must not be \c NULL.
 * \param handle      The Kunquat Handle -- must not be \c NULL.
 */
void Mix_state_from_handle(Mix_state* mix_state, kqt_Handle* handle);


#endif // MIX_STATE_H


