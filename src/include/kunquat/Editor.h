

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
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


