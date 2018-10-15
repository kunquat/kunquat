

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
 * Flags to specify what data to keep in the Kunquat Module for direct access.
 */
typedef enum
{
    KQTFILE_KEEP_NONE           = 0,
    KQTFILE_KEEP_RENDER_DATA    = 1 << 0,
    KQTFILE_KEEP_PLAYER_DATA    = 1 << 1,
    KQTFILE_KEEP_INTERFACE_DATA = 1 << 2,
    KQTFILE_KEEP_ALL_DATA =
        KQTFILE_KEEP_RENDER_DATA |
        KQTFILE_KEEP_PLAYER_DATA |
        KQTFILE_KEEP_INTERFACE_DATA,
} Kqtfile_keep_flags;


/**
 * Create a new Kunquat Handle from a Kunquat module file.
 *
 * This is a convenience function for most applications that do not need
 * special functionality during the loading of a Kunquat module.
 *
 * \param path           The path to an existing Kunquat module file
 *                       -- should be valid.
 * \param thread_count   The number of threads used for loading -- must be
 *                       > \c 0 and <= \c KQT_THREADS_MAX.
 *
 * \return   The new Kunquat Handle if successful. Otherwise, \c 0 is returned
 *           and the Kunquat file error is set accordingly. Additionally, the
 *           generic Kunquat Handle error may be set if the file contents were
 *           invalid.
 */
kqt_Handle kqtfile_load_module(const char* path, int thread_count);


/**
 * Create a new Kunquat Module with an associated Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- should be valid.
 *
 * \return   The new Kunquat Module if successful. Otherwise, \c 0 is returned
 *           and the Kunquat file error is set accordingly.
 */
kqt_Module kqt_new_Module_with_handle(kqt_Handle handle);


/**
 * Get human-readable error message from the Kunquat module.
 *
 * \param module   The Kunquat Module, or \c 0 if retrieving error information
 *                 that is not necessarily associated with an existing Kunquat
 *                 Module.
 *
 * \return   The latest error message. This is an empty string if no error has
 *           occurred.
 */
const char* kqt_Module_get_error(kqt_Module module);


/**
 * Clear error information of the Kunquat Module.
 *
 * Note that this function does not clear a possible error associated with the
 * Kunquat Handle of the module.
 *
 * \param module   The Kunquat Module, or \c 0 for clearing the generic error
 *                 message.
 */
void kqt_Module_clear_error(kqt_Module module);


/**
 * Tell Kunquat Module what data to keep for user access.
 *
 * \param module    The Kunquat Module -- should be valid.
 * \param flags     The keep flags -- should be a valid combination of flags.
 *
 * \return   \c 1 if successful, \c 0 on failure.
 */
int kqt_Module_set_keep_flags(kqt_Module module, Kqtfile_keep_flags flags);


/**
 * Open a Kunquat module file for reading.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   \c 1 on success. Otherwise, \c 0 is returned and Kunquat Module
 *           error is set accordingly.
 */
int kqt_Module_open_file(kqt_Module module, const char* path);


/**
 * Move a step forwards in Kunquat module loading.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   \c 1 if an entry was successfully read and there are more entries
 *           left, otherwise \c 0. The caller should check the result of
 *           \a kqt_Module_get_error(\a module) to determine whether an error
 *           occurred in case \c 0 is returned.
 */
int kqt_Module_load_step(kqt_Module module);


/**
 * Get module loading progress.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   The loading progress as a normalised floating-point number between
 *           \c 0 and \c 1, where \c 0 indicates the beginning and \c 1
 *           indicates the end of reading.
 */
double kqt_Module_get_loading_progress(kqt_Module module);


/**
 * Close a Kunquat module previously opened for reading.
 *
 * If there is no Kunquat module file currently opened for reading, this
 * function does nothing.
 *
 * \param module   The Kunquat Module -- should be valid.
 */
void kqt_Module_close_file(kqt_Module module);


/**
 * Get the number of kept entries in the Kunquat Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   The number of entries if successful, otherwise \c -1.
 */
long kqt_Module_get_kept_entry_count(kqt_Module module);


/**
 * Get the keys of kept entries in the Kunquat Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   The keys of kept entries if successful, otherwise \c NULL.
 */
const char** kqt_Module_get_kept_keys(kqt_Module module);


/**
 * Get the value sizes of kept entries in the Kunquat Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   The value sizes of kept entries if successful, otherwise \c NULL.
 *           The order of the sizes matches that of the keys returned by
 *           \a kqt_Module_get_kept_keys.
 */
const long* kqt_Module_get_kept_entry_sizes(kqt_Module module);


/**
 * Get the values of kept entries in the Kunquat Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   The values of kept entries if successful, otherwise \c NULL. The
 *           order of the entries matches that of the keys returned by
 *           \a kqt_Module_get_kept_keys.
 */
const char** kqt_Module_get_kept_entries(kqt_Module module);


/**
 * Free the kept entries in the Kunquat Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 *
 * \return   \c 1 if successful, or \c 0 if \a module is not valid.
 */
int kqt_Module_free_kept_entries(kqt_Module module);


/**
 * Free resources allocated for an existing Kunquat Module.
 *
 * Note that this function does not free the Kunquat Handle associated with the
 * Module.
 *
 * \param module   The Kunquat Module -- should be valid.
 */
void kqt_del_Module(kqt_Module module);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQTFILE_FILE_H


