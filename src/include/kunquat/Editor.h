

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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


#ifndef KQT_EDITOR_H
#define KQT_EDITOR_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>


/**
 * \defgroup Editor Editing Kunquat compositions
 *
 * \{
 *
 * \brief
 * This module describes the API for applications that modify Kunquat
 * compositions.
 */


/**
 * Initialises a read/write/commit Kunquat Handle from a composition state.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *                      See kqt_new_Handle for detailed explanation.
 * \param path          The path to the Kunquat composition state directory --
 *                      should not be \c NULL.
 *
 * \return   The new read/write/commit Kunquat Handle if successful, otherwise
 *           \c NULL  (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_rwc(long buffer_size, char* path);


/**
 * Commits changes made to the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_commit(kqt_Handle* handle);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_EDITOR_H


