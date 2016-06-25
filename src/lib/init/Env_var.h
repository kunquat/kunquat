

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ENV_VAR_H
#define KQT_ENV_VAR_H


#include <string/Streader.h>
#include <Value.h>

#include <stdlib.h>


/**
 * An environment variable.
 *
 * It is possible to cast a valid Env_var* to a char*. As a result, strcmp is
 * a valid sorting function for Environment variables.
 */
typedef struct Env_var Env_var;


/**
 * Create a new Environment variable.
 *
 * \param type   The type of the variable -- must be \c VALUE_TYPE_BOOL,
 *               \c VALUE_TYPE_INT, \c VALUE_TYPE_FLOAT  or \c VALUE_TYPE_TSTAMP.
 * \param name   The name of the variable -- must be a valid variable name.
 *
 * \return   The new Environment variable if successful, or \c NULL if memory
 *           allocation failed.
 */
Env_var* new_Env_var(Value_type type, const char* name);


/**
 * Create a new Environment variable from a textual description.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Environment variable if successful, otherwise \c NULL.
 */
Env_var* new_Env_var_from_string(Streader* sr);


/**
 * Return the type of the Environment variable.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   The type of the variable.
 */
Value_type Env_var_get_type(const Env_var* var);


/**
 * Return the name of the Environment variable.
 *
 * Note: It is also possible to get the name by casting the Environment
 * variable directly to char*.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   The name of the variable.
 */
const char* Env_var_get_name(const Env_var* var);


/**
 * Set the initial value of the Environment variable.
 *
 * \param var     The Environment variable -- must not be \c NULL.
 * \param value   The value to be set -- must not be \c NULL and must match
 *                the type of the variable.
 */
void Env_var_set_value(Env_var* var, const Value* value);


/**
 * Return the value of the Environment variable.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   A reference to the value of the variable. This is never \c NULL.
 */
const Value* Env_var_get_value(const Env_var* var);


/**
 * Destroy an existing Environment variable.
 *
 * \param var   The Environment variable, or \c NULL.
 */
void del_Env_var(Env_var* var);


#endif // KQT_ENV_VAR_H


