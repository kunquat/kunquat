

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_AU_EXPRESSIONS_H
#define KQT_AU_EXPRESSIONS_H


#include <decl.h>
#include <string/Streader.h>

#include <stdlib.h>


/**
 * Create a new Audio unit expression collection.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Audio unit expressions if successful, otherwise \c NULL.
 */
Au_expressions* new_Au_expressions(Streader* sr);


/**
 * Get the default note expression.
 *
 * \param ae    The Audio unit expressions -- must not be \c NULL.
 *
 * \return   The default expression name. This is never \c NULL.
 */
const char* Au_expressions_get_default_note_expr(const Au_expressions* ae);


/**
 * Get a processor filter in the Audio unit expression collection.
 *
 * \param ae     The Audio unit expressions -- must not be \c NULL.
 * \param name   The expression name -- must not be \c NULL.
 *
 * \return   The processor filter if one exists, otherwise \c NULL.
 */
const Param_proc_filter* Au_expressions_get_proc_filter(
        const Au_expressions* ae, const char* name);


/**
 * Destroy existing Audio unit expressions.
 *
 * \param ae   The Audio unit expressions, or \c NULL.
 */
void del_Au_expressions(Au_expressions* ae);


#endif // KQT_AU_EXPRESSIONS_H


