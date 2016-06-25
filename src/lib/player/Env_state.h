

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ENV_STATE_H
#define KQT_ENV_STATE_H


#include <init/Env_var.h>
#include <init/Environment.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Env_state Env_state;


/**
 * Create a new Environment state.
 *
 * \param env   The Environment -- must not be \c NULL.
 *
 * \return   The new Environment state if successful, or \c NULL if memory
 *           allocation failed.
 */
Env_state* new_Env_state(const Environment* env);


/**
 * Allocate state space for the Environment state.
 *
 * \param estate   The Environment state -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Env_state_refresh_space(Env_state* estate);


/**
 * Retrieve a variable from the Environment state.
 *
 * \param estate   The Environment state -- must not be \c NULL.
 * \param name     The variable name -- must not be \c NULL.
 *
 * \return   The variable if found, otherwise \c NULL.
 */
Env_var* Env_state_get_var(const Env_state* estate, const char* name);


/**
 * Reset the Environment state.
 *
 * \param estate   The Environment state -- must not be \c NULL.
 */
void Env_state_reset(Env_state* estate);


/**
 * Destroy an existing Environment state.
 *
 * \param estate   The Environment state, or \c NULL.
 */
void del_Env_state(Env_state* estate);


#endif // KQT_ENV_STATE_H


