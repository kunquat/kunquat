

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


#include <Tstamp.h>


/**
 * Iterates over triggers in column groups.
 */
typedef struct Cgiter
{
    Tstamp ref_pos;     // Current reference position inside a pattern.
} Cgiter;


/**
 * Initialises Cgiter.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 */
void Cgiter_init(Cgiter* cgiter);


