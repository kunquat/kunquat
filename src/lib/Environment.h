

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


#ifndef K_ENVIRONMENT_H
#define K_ENVIRONMENT_H


#include <stdlib.h>

#include <containers/AAtree.h>
#include <Env_var.h>
#include <Streader.h>


/**
 * A collection of externally modifiable state information.
 */
typedef struct Environment Environment;


typedef struct Environment_iter
{
    AAiter iter;
    const char* next;
} Environment_iter;


#define ENVIRONMENT_ITER_AUTO (&(Environment_iter){ { .tree = NULL }, NULL })


/**
 * Initialises an Environment iterator.
 *
 * \param iter   The Environment iterator -- must not be \c NULL.
 * \param env    The Environment -- must not be \c NULL.
 *
 * \return   The parameter \a iter.
 */
Environment_iter* Environment_iter_init(
        Environment_iter* iter,
        const Environment* env);


/**
 * Gets the next Environment variable name from the iterator.
 *
 * \param iter   The Environment iterator -- must not be \c NULL.
 *
 * \return   The next name, or \c NULL if reached the end of the environment.
 */
const char* Environment_iter_get_next_name(Environment_iter* iter);


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
 * \param env   The Environment -- must not be \c NULL.
 * \param sr    The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Environment_parse(Environment* env, Streader* sr);


/**
 * Gets a variable from the Environment.
 *
 * \param env    The Environment -- must not be \c NULL.
 * \param name   The variable name -- must not be \c NULL.
 *
 * \return   The variable if found, otherwise \c NULL.
 */
const Env_var* Environment_get(const Environment* env, const char* name);


/**
 * Destroys an existing Environment.
 *
 * \param env   The Environment, or \c NULL.
 */
void del_Environment(Environment* env);


#endif // K_ENVIRONMENT_H


