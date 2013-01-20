

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MANIFEST_H
#define K_MANIFEST_H


#include <stdbool.h>

#include <File_base.h>


/**
 * Reads a default manifest key.
 *
 * \param str     The string containing a JSON dictionary, or empty string
 *                or \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if a valid dictionary was found, otherwise \c false.
 *           \a state must be explicitly checked for parse errors.
 */
bool read_default_manifest(char* str, Read_state* state);


#endif // K_MANIFEST_H


