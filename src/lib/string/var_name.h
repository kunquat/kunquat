

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VAR_NAME_H
#define K_VAR_NAME_H


#include <stdbool.h>
#include <stdlib.h>


/**
 * Check if a string is a valid variable name.
 *
 * \param str   The string -- must not be \c NULL.
 *
 * \return   \c true if \a str is a valid variable name, otherwise \c false.
 */
bool is_valid_var_name(const char* str);


#endif // K_VAR_NAME_H


