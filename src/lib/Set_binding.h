

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SET_BINDING_H
#define K_SET_BINDING_H


#include <stdlib.h>
#include <string.h>

#include <Event_names.h>
#include <File_base.h>


/**
 * A binding from a variable setter to events.
 *
 * Instances of this class may be compared using strcmp.
 */
typedef struct Set_binding Set_binding;


/**
 * Creates a new Set binding from a JSON string.
 *
 * \param str     A reference to the JSON string. str and *str must not
 *                be \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Set binding if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Set_binding* new_Set_binding_from_string(char** str,
                                         Event_names* names,
                                         Read_state* state);


/**
 * Destroys an existing Set binding.
 *
 * \param sb   The Set binding, or \c NULL.
 */
void del_Set_binding(Set_binding* sb);


#endif // K_SET_BINDING_H


