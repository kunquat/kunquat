

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CHANNEL_PROC_STATE_H
#define K_CHANNEL_PROC_STATE_H


#include <string/Streader.h>
#include <Tstamp.h>
#include <Value.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * A dictionary for processor parameters specific to a Channel.
 */
typedef struct Channel_proc_state Channel_proc_state;


/**
 * Create a new Channel proc state.
 *
 * \return   The new Channel proc state if successful, or \c NULL if memory
 *           allocation failed.
 */
Channel_proc_state* new_Channel_proc_state(void);


/**
 * Allocate memory for a list of keys.
 *
 * \param cpstate   The Channel proc state -- must not be \c NULL.
 * \param sr        The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Channel_proc_state_alloc_keys(Channel_proc_state* cpstate, Streader* sr);


/**
 * Modify an existing parameter value.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must end with one of
 *                Processor parameter suffixes.
 * \param value   The new value -- must not be \c NULL.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
bool Channel_proc_state_modify_value(
        Channel_proc_state* cpstate,
        const char* key,
        const Value* value);


/**
 * Retrieve a reference to a boolean value.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The boolean value, or \c NULL if \a key does not match a
 *           boolean entry.
 */
const bool* Channel_proc_state_get_bool(
        const Channel_proc_state* cpstate,
        const char* key);


/**
 * Retrieve a reference to an integer value.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The integer value, or \c NULL if \a key does not match an
 *           integer entry.
 */
const int64_t* Channel_proc_state_get_int(
        const Channel_proc_state* cpstate,
        const char* key);


/**
 * Retrieve a reference to a floating-point value.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The float value, or \c NULL if \a key does not match a
 *           float entry.
 */
const double* Channel_proc_state_get_float(
        const Channel_proc_state* cpstate,
        const char* key);


/**
 * Retrieve a reference to a timestamp value.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The Tstamp value, or \c NULL if \a key does not match a
 *           timestamp entry.
 */
const Tstamp* Channel_proc_state_get_tstamp(
        const Channel_proc_state* cpstate,
        const char* key);


/**
 * Clear a Channel proc state.
 *
 * \param state   The Channel proc state -- must not be \c NULL.
 */
void Channel_proc_state_clear(Channel_proc_state* cpstate);


/**
 * Destroy an existing Channel proc state.
 *
 * \param state   The Channel proc state, or \c NULL.
 */
void del_Channel_proc_state(Channel_proc_state* cpstate);


#endif // K_CHANNEL_PROC_STATE_H


