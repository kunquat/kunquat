

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


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/limits.h>
#include <kunquat/frame.h>


typedef struct kqt_Context kqt_Context;


/**
 * Creates a new Kunquat Context.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *
 * \return   The new Kunquat Context if successful, or \c NULL if memory
 *           allocation failed.
 */
kqt_Context* kqt_new_Context(long buffer_size);


/**
 * Gets error information from the Kunquat Context.
 *
 * \param context   The Context, or \c NULL if retrieving error information
 *                  that is not associated with a Kunquat Context.
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


#ifdef __cplusplus
}
#endif


#endif // KQT_CONTEXT_H


