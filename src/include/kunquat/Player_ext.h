

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


#ifndef KQT_PLAYER_EXT_H
#define KQT_PLAYER_EXT_H


#include <kunquat/Context.h>
#include <kunquat/Player.h>


/**
 * Decomposes a time representation into parts.
 *
 * \param time          The time representation -- should not be \c NULL.
 * \param subsong       Location where the Subsong number will be stored (optional).
 * \param section       Location where the section number will be stored (optional).
 * \param beats         Location where the beat count will be stored (optional).
 * \param remainder     Location where the beat remainder will be stored (optional).
 * \param nanoseconds   Location where the nanosecond count will be stored (optional).
 */
int kqt_unwrap_time(char* time, 
                    int* subsong,
                    int* section,
                    long long* beats,
                    long* remainder,
                    long long* nanoseconds);


/**
 * Sets the position to be played.
 *
 * Any notes that were being played will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param context    The Context -- should not be \c NULL.
 * \param position   The new position -- should not be \c NULL.
 *                   The position is a string with the format
 *                   "<subsong>[/<section>[/<timestamp>]][+<nanoseconds>]
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Context_set_position(kqt_Context* context, char* position);


/**
 * Gets the current position of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The position description, or \c NULL if \a context is invalid.
 */
char* kqt_Context_get_position(kqt_Context* context);


/**
 * Tells whether mixing of the Kunquat Context has reached the end.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   \c 1 if the end has been reached, otherwise \c 0.
 */
int kqt_Context_end_reached(kqt_Context* context);


/**
 * Gets the total number of frames mixed after the last position change.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of frames.
 */
long long kqt_Context_get_frames_mixed(kqt_Context* context);


/**
 * Gets the current tempo in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The current tempo.
 */
double kqt_Context_get_tempo(kqt_Context* context);


/**
 * Gets the maximum number of simultaneous Voices used.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of Voices.
 */
int kqt_Context_get_voice_count(kqt_Context* context);


/**
 * Gets the minimum amplitude value encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The minimum amplitude value, or \c INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Context_get_min_amplitude(kqt_Context* context, int buffer);


/**
 * Gets the maximum amplitude value encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The maximum amplitude value, or \c -INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Context_get_max_amplitude(kqt_Context* context, int buffer);


/**
 * Gets the number of clipped frames encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The number of clipped frames.
 */
long kqt_Context_get_clipped(kqt_Context* context, int buffer);


/**
 * Resets mixing state statistics.
 *
 * The values that will be reset are number of Voices used, peak amplitude
 * values and counts of clipped frames. Thus, it doesn't affect the playback.
 *
 * \param context   The Context -- should not be \c NULL.
 */
void kqt_Context_reset_stats(kqt_Context* context);


#endif // KQT_PLAYER_EXT_H


