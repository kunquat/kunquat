

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


#include <kunquat/Context.h>


/**
 * Sets the buffer size of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param size      The new buffer size -- should be > \c 0.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 *           Note: If memory allocation fails, mixing is still possible but
 *           only with min{old_size, new_size} frames at a time. However, it
 *           may be a good idea to just give up in this case.
 */
int kqt_Context_set_buffer_size(kqt_Context* context, long size);


/**
 * Gets the buffer size of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The size of a buffer in frames.
 */
long kqt_Context_get_buffer_size(kqt_Context* context);


/**
 * Does mixing according to the state of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 * \param freq      The mixing frequency -- should be > \c 0.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
long kqt_Context_mix(kqt_Context* context, long nframes, long freq);


/**
 * Gets the length of the Kunquat Context in nanoseconds.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
long long kqt_Context_get_length_ns(kqt_Context* context);


/**
 * Gets the number of mixing buffers in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of buffers, or \c 0 if \a context == \c NULL.
 */
int kqt_Context_get_buffer_count(kqt_Context* context);


/**
 * Gets the mixing buffers in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The buffers, or \c NULL if \a context == \c NULL.
 */
kqt_frame** kqt_Context_get_buffers(kqt_Context* context);


/**
 * Sets the position to be played in nanoseconds.
 *
 * Any notes that were being played will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param context       The Context -- should not be \c NULL.
 * \param nanoseconds   The number of nanoseconds from the beginning --
 *                      should not be negative.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Context_set_position_ns(kqt_Context* context, long long nanoseconds);


/**
 * Gets the current position in nanoseconds.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The amount of nanoseconds mixed since the start of mixing.
 */
long long kqt_Context_get_position_ns(kqt_Context* context);


#endif // KQT_PLAYER_H


