

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
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
#include <stdlib.h>

#include <string/Streader.h>


/**
 * Reads a default manifest key.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if a valid dictionary was found, otherwise \c false.
 *           \a state must be explicitly checked for parse errors.
 */
bool read_default_manifest(Streader* sr);


#endif // K_MANIFEST_H


