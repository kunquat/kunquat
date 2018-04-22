

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQTFILE_FILE_H
#define KQTFILE_FILE_H


#include <kunquat/Handle.h>

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup File Kunquat File Utilities
 * \{
 *
 * \brief
 * This module contains a simple interface for accessing Kunquat files.
 *
 * The header \c Handle.h contains an API for accessing Kunquat Handles.
 */


/**
 * The identifier for accessing a Kunquat module.
 */
typedef int kqt_Module;


/**
 * Create a new Kunquat Handle from a Kunquat module file.
 *
 * \param path   The path to an existing Kunquat module file
 *               -- should be valid.
 *
 * \return   The new Kunquat Handle if successful. Otherwise, \c 0 is returned
 *           and the Kunquat file error is set accordingly. Additionally, the
 *           generic Kunquat Handle error may be set if the file contents were
 *           invalid.
 */
kqt_Handle kqtfile_load_module(const char* path);


/**
 * Get human-readable error message from the Kunquat file interface.
 *
 * \param module   The Kunquat module, or \c 0 if retrieving error information
 *                 that is not necessarily associated with an existing Kunquat
 *                 module. For now, this should always be \c 0.
 *
 * \return   The latest error message. This is an empty string if no error has
 *           occurred.
 */
const char* kqtfile_get_error(kqt_Module module);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQTFILE_FILE_H


