

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


#ifndef K_ENVIRONMENT_H
#define K_ENVIRONMENT_H


#include <stdlib.h>

#include <Env_var.h>
#include <File_base.h>


/**
 * A collection of state information.
 */
typedef struct Environment Environment;


/**
 * Creates a new Environment.
 *
 * \return   The new Environment if successful, or \c NULL if memory
 *           allocation failed.
 */
Environment* new_Environment(void);


/**
 * Parses the Environment from a string.
 *
 * \param env     The Environment -- must not be \c NULL.
 * \param str     The textual description, or \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will not
 *           be modified if memory allocation failed.
 */
bool Environment_parse(Environment* env, char* str, Read_state* state);


/**
 * Resets the Environment.
 *
 * \param env   The Environment -- must not be \c NULL.
 */
void Environment_reset(Environment* env);


/**
 * Gets a variable from the Environment.
 *
 * \param env    The Environment -- must not be \c NULL.
 * \param name   The variable name -- must not be \c NULL.
 *
 * \return   The variable if found, otherwise \c NULL.
 */
Env_var* Environment_get(Environment* env, char* name);


/**
 * Destroys an existing Environment.
 *
 * \param env   The Environment, or \c NULL.
 */
void del_Environment(Environment* env);


#endif // K_ENVIRONMENT_H


