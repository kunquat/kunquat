

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
 * Initialises a Kunquat Handle from a composition state directory.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *                      See kqt_new_Handle for detailed explanation.
 * \param path          The path to the Kunquat composition state directory --
 *                      should not be \c NULL.
 *
 * \return   The new composition state Kunquat Handle if successful, otherwise
 *           \c NULL  (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_state_init(long buffer_size, char* path);


/**
 * Commits changes made to the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_commit(kqt_Handle* handle);


/**
 * Inserts an Event inside the Kunquat Handle.
 *
 * This function only synchronises the player state! The user must
 * additionally call kqt_Handle_write_column to actually make the
 * changes to the data in the composition state.
 *
 * \param handle      The Handle -- should not be \c NULL.
 * \param pattern     The number of the pattern -- should be >= \c 0 and
 *                    < \c KQT_PATTERNS_MAX.
 * \param column      The number of the column -- should be >= \c -1 and
 *                    < \c KQT_COLUMNS_MAX. The index \c -1 is the global
 *                    column.
 * \param beat        The index of the beat -- should be >= \c 0.
 * \param remainder   The remainder of the beat -- should be >= \c 0 and
 *                    < \c KQT_RELTIME_BEAT.
 * \param index       The 0-based index of the Event in the row.
 * \param type        The type of the Event.
 * \param fields      The fields of the Event -- should correspond to the
 *                    type description of the Event.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_insert_event(kqt_Handle* handle,
                            int pattern,
                            int column,
                            long long beat,
                            long remainder,
                            int index,
                            Event_type type,
                            char* fields);


/**
 * Writes a column into the composition state.
 *
 * \param handle      The Handle -- should not be \c NULL.
 * \param pattern     The number of the pattern -- should be >= \c 0 and
 *                    < \c KQT_PATTERNS_MAX.
 * \param column      The number of the column -- should be >= \c -1 and
 *                    < \c KQT_COLUMNS_MAX. The index \c -1 is the global
 *                    column.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_write_column(kqt_Handle* handle,
                            int pattern,
                            int column);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_EDITOR_H


