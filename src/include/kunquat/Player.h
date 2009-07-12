

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


#ifndef KQT_PLAYER_H
#define KQT_PLAYER_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>


/**
 * Sets the buffer size of the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param size     The new buffer size -- should be > \c 0.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 *           Note: If memory allocation fails, mixing is still possible but
 *           only with min{old_size, new_size} frames at a time. However, it
 *           may be a good idea to just give up in this case.
 */
int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size);


/**
 * Gets the buffer size of the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The size of a buffer in frames.
 */
long kqt_Handle_get_buffer_size(kqt_Handle* handle);


/**
 * Does mixing according to the state of the Kunquat Handle.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 * \param freq      The mixing frequency -- should be > \c 0.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
long kqt_Handle_mix(kqt_Handle* handle, long nframes, long freq);


/**
 * Gets the length of the Kunquat Handle in nanoseconds.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
long long kqt_Handle_get_duration(kqt_Handle* handle);


/**
 * Gets the number of mixing buffers in the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The number of buffers, or \c 0 if \a handle == \c NULL.
 */
int kqt_Handle_get_buffer_count(kqt_Handle* handle);


/**
 * Gets the mixing buffers in the Kunquat Handle.
 *
 * The returned value \a bufs is an array of buffers where \a bufs[0]
 * contains the buffer of the first output channel (left channel in stereo),
 * \a bufs[1] contains the buffer of the second output channel, and so on.
 * \a bufs[kqt_Handle_get_buffer_count(\a handle)] is always \c NULL.
 * Each buffer contains kqt_Handle_get_buffer_size(\a handle) amplitude
 * values of type \a kqt_frame.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The buffers, or \c NULL if \a handle == \c NULL.
 */
kqt_frame** kqt_Handle_get_buffers(kqt_Handle* handle);


/**
 * Sets the position to be played.
 *
 * Any notes that were being played will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param handle        The Handle -- should not be \c NULL.
 * \param subsong       The Subsong number -- should be >= \c -1 and
 *                      < \c KQT_SUBSONGS_MAX (\c -1 contains all Subsongs).
 * \param nanoseconds   The number of nanoseconds from the beginning --
 *                      should not be negative.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_seek(kqt_Handle* handle, int subsong, long long nanoseconds);


/**
 * Gets the current position in nanoseconds.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The amount of nanoseconds mixed since the start of mixing.
 */
long long kqt_Handle_tell(kqt_Handle* handle);


#ifdef __cplusplus
}
#endif


#endif // KQT_PLAYER_H


