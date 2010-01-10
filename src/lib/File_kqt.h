

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


#ifndef K_FILE_KQT_H
#define K_FILE_KQT_H


#include <stdbool.h>

#include <Handle_r.h>


/**
 * Opens a .kqt file inside the Kunquat Handle.
 *
 * \param handle_r   The read-only Kunquat Handle -- must not be \c NULL.
 * \param path       The path of the .kqt file -- must not be \c NULL.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           handle error is set to describe the error.
 */
bool File_kqt_open(Handle_r* handle_r, const char* path);


#endif // K_FILE_KQT_H


