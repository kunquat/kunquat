

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


#ifndef K_AU_CONTROL_VARS_H
#define K_AU_CONTROL_VARS_H


#include <stdlib.h>

#include <string/Streader.h>


/**
 * An abstraction layer for controlling internal device parameters during playback.
 */
typedef struct Au_control_vars Au_control_vars;


/**
 * Create new Audio unit control variables.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Audio unit control variables if successful, otherwise \c NULL.
 */
Au_control_vars* new_Au_control_vars(Streader* sr);


/**
 * Destroy existing Audio unit control variables.
 *
 * \param acv   The Audio unit control variables, or \c NULL.
 */
void del_Au_control_vars(Au_control_vars* acv);


#endif // K_AU_CONTROL_VARS_H


