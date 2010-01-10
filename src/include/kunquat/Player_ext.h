

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef KQT_PLAYER_EXT_H
#define KQT_PLAYER_EXT_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>
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
 *
 * \return   \c 1 if \a time is a valid time representation, otherwise \c 0.
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
 * The position is specified as a string with the following format:
 *
 * <subsong>[/<section>[/<timestamp>]][+<nanoseconds>]
 *
 * where
 *
 * <subsong> is the subsong number
 * <section> is the section number
 * <timestamp> is <beat>,<rem>
 *                where <beat> is the beat count
 *                      <rem> is the beat remainder (see kunquat/Reltime.h)
 * <nanoseconds> is the non-negative offset in nanoseconds.
 *
 * Important: Because the position of a section and a timestamp in time
 *            elapsed from the beginning is ambiguous, the function
 *            \a kqt_Handle_tell shouldn't be used after a jump to a section
 *            or timestamp.
 *
 * \param handle     The Handle -- should not be \c NULL.
 * \param position   The new position -- should not be \c NULL.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_set_position_desc(kqt_Handle* handle, char* position);


/**
 * Gets the current position of the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The position description, or \c NULL if \a handle is invalid.
 */
char* kqt_Handle_get_position_desc(kqt_Handle* handle);


/**
 * Tells whether mixing of the Kunquat Handle has reached the end.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   \c 1 if the end has been reached, otherwise \c 0.
 */
int kqt_Handle_end_reached(kqt_Handle* handle);


/**
 * Gets the total number of frames mixed after the last position change.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The number of frames.
 */
long long kqt_Handle_get_frames_mixed(kqt_Handle* handle);


/**
 * Gets the current tempo in the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The current tempo.
 */
double kqt_Handle_get_tempo(kqt_Handle* handle);


/**
 * Gets the maximum number of simultaneous Voices used.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The number of Voices.
 */
int kqt_Handle_get_voice_count(kqt_Handle* handle);


/**
 * Gets the minimum amplitude value encountered.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param buffer   The buffer index (0 for left, 1 for right).
 *
 * \return   The minimum amplitude value, or \c INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Handle_get_min_amplitude(kqt_Handle* handle, int buffer);


/**
 * Gets the maximum amplitude value encountered.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param buffer   The buffer index (0 for left, 1 for right).
 *
 * \return   The maximum amplitude value, or \c -INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Handle_get_max_amplitude(kqt_Handle* handle, int buffer);


/**
 * Gets the number of clipped frames encountered.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param buffer   The buffer index (0 for left, 1 for right).
 *
 * \return   The number of clipped frames.
 */
long long kqt_Handle_get_clipped(kqt_Handle* handle, int buffer);


/**
 * Resets mixing state statistics.
 *
 * The values that will be reset are number of Voices used, peak amplitude
 * values and counts of clipped frames. Thus, it doesn't affect the playback.
 *
 * \param handle   The Handle -- should not be \c NULL.
 */
void kqt_Handle_reset_stats(kqt_Handle* handle);


#ifdef __cplusplus
}
#endif


#endif // KQT_PLAYER_EXT_H


