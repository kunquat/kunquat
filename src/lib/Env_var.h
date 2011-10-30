

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


#ifndef K_ENV_VAR_H
#define K_ENV_VAR_H


#include <stdbool.h>
#include <stdint.h>

#include <File_base.h>
#include <Real.h>
#include <Reltime.h>


/**
 * The possible types of an environment variable.
 */
typedef enum
{
    ENV_VAR_BOOL,       ///< bool
    ENV_VAR_INT,        ///< int64_t
    ENV_VAR_FLOAT,      ///< double
    ENV_VAR_REAL,       ///< Real
    ENV_VAR_RELTIME,    ///< Reltime
} Env_var_type;


/**
 * An environment variable.
 *
 * It is possible to cast a valid Env_var* to a char*. As a result, strcmp is
 * a valid sorting function for Environment variables.
 */
typedef struct Env_var Env_var;


#define ENV_VAR_NAME_MAX 32
#define ENV_VAR_INIT_CHARS "abcdefghijklmnopqrstuvwxyz_"
#define ENV_VAR_CHARS ENV_VAR_INIT_CHARS "0123456789"


/**
 * Creates a new Environment variable.
 *
 * \param type   The variable type -- must be valid.
 * \param name   The variable name -- must not be \c NULL.
 *
 * \return   The new Environment variable if successful, or \c NULL if memory
 *           allocation failed.
 */
Env_var* new_Env_var(Env_var_type type, const char* name);


/**
 * Creates a new Environment variable from a textual description.
 *
 * \param str     A reference to the textual description
 *                -- must not be \c NULL or point to a \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Environment variable if successful, otherwise \c NULL.
 *           \a state will not be modified if memory allocation failed.
 */
Env_var* new_Env_var_from_string(char** str, Read_state* state);


/**
 * Returns the type of the Environment variable.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   The type of the variable.
 */
Env_var_type Env_var_get_type(Env_var* var);


/**
 * Returns the name of the Environment variable.
 *
 * Note: It is also possible to get the name by casting the Environment
 * variable directly to char*.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   The name of the variable.
 */
char* Env_var_get_name(Env_var* var);


/**
 * Sets the initial value of the Environment variable.
 *
 * \param var     The Environment variable -- must not be \c NULL.
 * \param value   The value to be set -- must not be \c NULL and must match
 *                the type of the variable.
 */
void Env_var_set_value(Env_var* var, void* value);


/**
 * Modifies the value of the Environment variable.
 *
 * \param var     The Environment variable -- must not be \c NULL.
 * \param value   The value to be set -- must not be \c NULL and must match
 *                the type of the variable.
 */
void Env_var_modify_value(Env_var* var, void* value);


/**
 * Resets the value of the Environment variable.
 *
 * \param var     The Environment variable -- must not be \c NULL.
 */
void Env_var_reset(Env_var* var);


/**
 * Returns the value of the Environment variable.
 *
 * \param var   The Environment variable -- must not be \c NULL.
 *
 * \return   A reference to the value of the variable. This is never \c NULL.
 */
void* Env_var_get_value(Env_var* var);


/**
 * Destroys an existing Environment variable.
 *
 * \param var   The Environment variable, or \c NULL.
 */
void del_Env_var(Env_var* var);


#endif // K_ENV_VAR_H


