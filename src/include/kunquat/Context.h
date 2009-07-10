

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


#ifndef KQT_CONTEXT_H
#define KQT_CONTEXT_H


#include <stdint.h>

#include <kunquat/frame.h>


#define MAX_VOICES (1024)


typedef struct kqt_Context kqt_Context;


/**
 * Creates a new Kunquat Context.
 *
 * \param buf_count          The number of buffers used for mixing. Currently,
 *                           this can be 1 (mono) or 2 (stereo).
 * \param buf_size           The size of the mixing buffer.
 * \param voice_count        The number of Voices used for mixing.
 * \param event_queue_size   The size of the Event queue for each Column.
 *
 * \return   The new Kunquat Context if successful, or \c NULL if memory
 *           allocation failed.
 */
kqt_Context* kqt_new_Context(int buf_count,
                             long buf_size,
                             short voice_count,
                             short event_queue_size);


/**
 * Gets error information from the Kunquat Context.
 *
 * \param context   The Context, or \c NULL if retrieving error information
 *                  not associated with a Kunquat Context.
 *
 * \return   The error message. This is an empty string if no error has
 *           occurred.
 */
char* kqt_Context_get_error(kqt_Context* context);


/**
 * Loads contents of a Kunquat composition file or directory.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param path      The path to the Kunquat composition file or directory
 *                  -- should not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
int kqt_Context_load(kqt_Context* context, char* path);


/**
 * Gets the length of a Subsong in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 *
 * \return   The length of the Subsong, or \c -1 if arguments were invalid.
 */
int kqt_Context_get_subsong_length(kqt_Context* context, int subsong);


/**
 * Destroys an existing Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 */
void kqt_del_Context(kqt_Context* context);


#endif // KQT_CONTEXT_H


