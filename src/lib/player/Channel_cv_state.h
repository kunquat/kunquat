

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


#ifndef K_CHANNEL_CV_STATE_H
#define K_CHANNEL_CV_STATE_H


#include <stdlib.h>

#include <kunquat/limits.h>
#include <Value.h>


/**
 * A dictionary for audio unit control variable states in a Channel.
 */
typedef struct Channel_cv_state Channel_cv_state;


/**
 * Create a new Channel control variable state.
 *
 * \return   The new Channel control variable state if successful, or \c NULL
 *           if memory allocation failed.
 */
Channel_cv_state* new_Channel_cv_state(void);


/**
 * Add a control variable entry to the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL and must be
 *                   shorter than \c KQT_VAR_NAME_MAX characters.
 * \param var_type   The variable type -- must be \c VALUE_TYPE_BOOL,
 *                   \c VALUE_TYPE_INT, \c VALUE_TYPE_FLOAT or \c VALUE_TYPE_TSTAMP.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Channel_cv_state_add_entry(
        Channel_cv_state* state, const char* var_name, Value_type var_type);


/**
 * Set a value of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param value      The new value -- must not be \c NULL.
 *
 * \return   \c true if the new value was actually set, or \c false if \a state
 *           does not contain an entry with matching variable name and value type.
 */
bool Channel_cv_state_set_value(
        Channel_cv_state* state, const char* var_name, const Value* value);


/**
 * Get a value of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param var_type   The variable type -- must be \c VALUE_TYPE_BOOL,
 *                   \c VALUE_TYPE_INT, \c VALUE_TYPE_FLOAT or \c VALUE_TYPE_TSTAMP.
 *
 * \return   The stored value if one exists, otherwise \c NULL.
 */
const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state, const char* var_name, Value_type var_type);


/**
 * Reset all variables in the Channel control variable state.
 *
 * \param state   The Channel control variable state -- must not be \c NULL.
 */
void Channel_cv_state_reset(Channel_cv_state* state);


/**
 * Destroy an existing Channel control variable state.
 *
 * \param state   The Channel control variable state, or \c NULL.
 */
void del_Channel_cv_state(Channel_cv_state* state);


#endif // K_CHANNEL_CV_STATE_H


