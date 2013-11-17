

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CHANNEL_GEN_STATE_H
#define K_CHANNEL_GEN_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <Streader.h>
#include <Tstamp.h>
#include <Value.h>


/**
 * A dictionary for generator parameters specific to a Channel.
 */
typedef struct Channel_gen_state Channel_gen_state;


/**
 * Creates a new Channel gen state.
 *
 * \return   The new Channel gen state if successful, or \c NULL if memory
 *           allocation failed.
 */
Channel_gen_state* new_Channel_gen_state(void);


/**
 * Allocates memory for a list of keys.
 *
 * \param cgstate   The Channel gen state -- must not be \c NULL.
 * \param sr        The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Channel_gen_state_alloc_keys(Channel_gen_state* cgstate, Streader* sr);


/**
 * Modifies an existing parameter value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must end with one of
 *                Generator parameter suffixes.
 * \param value   The new value -- must not be \c NULL.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
bool Channel_gen_state_modify_value(
        Channel_gen_state* cgstate,
        const char* key,
        const Value* value);


/**
 * Retrieves a reference to a boolean value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The boolean value, or \c NULL if \a key does not match a
 *           boolean entry.
 */
const bool* Channel_gen_state_get_bool(
        const Channel_gen_state* cgstate,
        const char* key);


/**
 * Retrieves a reference to an integer value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The integer value, or \c NULL if \a key does not match an
 *           integer entry.
 */
const int64_t* Channel_gen_state_get_int(
        const Channel_gen_state* cgstate,
        const char* key);


/**
 * Retrieves a reference to a floating-point value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The float value, or \c NULL if \a key does not match a
 *           float entry.
 */
const double* Channel_gen_state_get_float(
        const Channel_gen_state* cgstate,
        const char* key);


/**
 * Retrieves a reference to a timestamp value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL.
 *
 * \return   The Tstamp value, or \c NULL if \a key does not match a
 *           timestamp entry.
 */
const Tstamp* Channel_gen_state_get_tstamp(
        const Channel_gen_state* cgstate,
        const char* key);


/**
 * Clears a Channel gen state.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 */
void Channel_gen_state_clear(Channel_gen_state* cgstate);


/**
 * Destroys an existing Channel gen state.
 *
 * \param state   The Channel gen state, or \c NULL.
 */
void del_Channel_gen_state(Channel_gen_state* cgstate);


#endif // K_CHANNEL_GEN_STATE_H


