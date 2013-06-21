

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

#include <Real.h>
#include <Tstamp.h>


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
 * Allocates memory for a key.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must end with one of
 *                Generator parameter suffixes.
 */
bool Channel_gen_state_set_key(Channel_gen_state* state, const char* key);


/**
 * Modifies an existing parameter value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must end with one of
 *                Generator parameter suffixes.
 * \param data    The new value -- must not be \c NULL.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
bool Channel_gen_state_modify_value(
        Channel_gen_state* state,
        const char* key,
        void* data);


/**
 * Retrieves a reference to a boolean value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must have the suffix
 *                ".jsonb".
 *
 * \return   The boolean value, or \c NULL if \a key doesn't exist.
 */
bool* Channel_gen_state_get_bool(
        Channel_gen_state* state,
        const char* key);


/**
 * Retrieves a reference to an integer value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must have the suffix
 *                ".jsoni".
 *
 * \return   The integer value, or \c NULL if \a key doesn't exist.
 */
int64_t* Channel_gen_state_get_int(
        Channel_gen_state* state,
        const char* key);


/**
 * Retrieves a reference to a floating-point value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must have the suffix
 *                ".jsonf".
 *
 * \return   The floating-point value, or \c NULL if \a key doesn't exist.
 */
double* Channel_gen_state_get_float(
        Channel_gen_state* state,
        const char* key);


/**
 * Retrieves a reference to a Real value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must have the suffix
 *                ".jsonr".
 *
 * \return   The Real value, or \c NULL if \a key doesn't exist.
 */
Real* Channel_gen_state_get_real(
        Channel_gen_state* state,
        const char* key);


/**
 * Retrieves a reference to a Tstamp value.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 * \param key     The key -- must not be \c NULL and must have the suffix
 *                ".jsont".
 *
 * \return   The Tstamp value, or \c NULL if \a key doesn't exist.
 */
Tstamp* Channel_gen_state_get_tstamp(
        Channel_gen_state* state,
        const char* key);


/**
 * Clears a Channel gen state.
 *
 * \param state   The Channel gen state -- must not be \c NULL.
 */
void Channel_gen_state_clear(Channel_gen_state* state);


/**
 * Destroys an existing Channel gen state.
 *
 * \param state   The Channel gen state, or \c NULL.
 */
void del_Channel_gen_state(Channel_gen_state* state);


#endif // K_CHANNEL_GEN_STATE_H


