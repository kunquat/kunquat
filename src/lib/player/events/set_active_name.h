

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SET_ACTIVE_NAME_H
#define K_SET_ACTIVE_NAME_H


#include <stdbool.h>

#include <player/Active_names.h>
#include <player/Event_type.h>
#include <player/General_state.h>
#include <Value.h>


/**
 * Sets the name of a variable subject to future modification.
 *
 * \param gstate   The General state -- must not be \c NULL.
 * \param cat      The active category -- must be valid.
 * \param type     The active variable type -- must be valid.
 * \param fields   The event fields.
 *
 * \return   \c true if and only if \a fields is valid.
 */
bool set_active_name(General_state* gstate,
                     Active_cat cat,
                     Active_type type,
                     Value* value);


#endif // K_SET_ACTIVE_NAME_H


