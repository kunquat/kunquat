

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DEVICE_PARAMS_H
#define K_DEVICE_PARAMS_H


#include <stdbool.h>
#include <stdint.h>

#include <File_base.h>
#include <Device_field.h>
#include <Real.h>
#include <Reltime.h>


typedef struct Device_params Device_params;


/**
 * Tells whether a given key is a Device parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a Device parameter key, otherwise \c false.
 */
bool key_is_device_param(const char* key);


/**
 * Tells whether a given key is a real-time modifiable Device parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a real-time modifiable Device parameter
 *           key, otherwise \c false.
 */
bool key_is_real_time_device_param(const char* key);


/**
 * Tells whether a given key is a text-mode Device parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a text-mode Device parameter key,
 *           otherwise \c false.
 */
bool key_is_text_device_param(const char* key);


/**
 * Creates new Device parameters.
 *
 * \return   The new Device parameters if successful, otherwise \c NULL.
 */
Device_params* new_Device_params(void);


/**
 * Allocates memory for a key.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_params_set_key(Device_params* params, const char* key);


/**
 * Marks a key to require explicit synchronisation on update.
 *
 * If the function succeeds, it also sets the Device parameters to require
 * synchronisation.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_params_set_slow_sync(Device_params* params, const char* key);


/**
 * Marks a key to no longer require explicit synchronisation on update.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 */
void Device_params_clear_slow_sync(Device_params* params, const char* key);


/**
 * Finds out whether any slow-sync parameters have changed.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 *
 * \return   \c true if explicit synchronisation is needed,
 *           otherwise \c false.
 */
bool Device_params_need_sync(Device_params* params);


/**
 * Retrieves a slow-sync key that has changed.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 *
 * \return   One of the slow-sync keys, or \c NULL if no such keys are left.
 */
const char* Device_params_get_slow_sync_key(Device_params* params);


/**
 * Sets the Device parameters synchronised.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 */
void Device_params_synchronised(Device_params* params);


/**
 * Parses the Device Event list.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param str      The textual description.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will _not_ be
 *           modified if memory allocation failed.
 */
//bool Device_params_parse_events(Device_params* params,
//                                char* str,
//                                Read_state* state);


/**
 * Parses a Device parameter value.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey with the i/ or c/ as
 *                 the first component.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length of the data -- must be >= \c 0.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will _not_ be
 *           modified if memory allocation failed.
 */
bool Device_params_parse_value(Device_params* params,
                               const char* key,
                               void* data,
                               long length,
                               Read_state* state);


/**
 * Modifies an existing Device parameter value.
 *
 * This function is used during playback.
 * Note that this function does not support sample files.
 * Also, the function will not modify keys that are marked to require explicit
 * synchronisation.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 * \param str      The new value as a string.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
bool Device_params_modify_value(Device_params* params,
                                const char* key,
                                char* str);


/**
 * Retrieves a reference to a boolean value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonb".
 *
 * \return   The boolean value, or \c NULL if \a key doesn't exist.
 */
bool* Device_params_get_bool(Device_params* params, const char* key);


/**
 * Retrieves a reference to an integer value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsoni".
 *
 * \return   The integer value, or \c NULL if \a key doesn't exist.
 */
int64_t* Device_params_get_int(Device_params* params, const char* key);


/**
 * Retrieves a reference to a floating point value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonf".
 *
 * \return   The floating point value, or \c NULL if \a key doesn't exist.
 */
double* Device_params_get_float(Device_params* params, const char* key);


/**
 * Retrieves a Real value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonr".
 *
 * \return   The Real value, or \c NULL if \a key doesn't exist.
 */
Real* Device_params_get_real(Device_params* params, const char* key);


/**
 * Retrieves a Reltime value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsont".
 *
 * \return   The Reltime value, or \c NULL if \a key doesn't exist.
 */
Reltime* Device_params_get_reltime(Device_params* params, const char* key);


/**
 * Retrieves a Sample from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".wv".
 *
 * \return   The Sample, or \c NULL if \a key doesn't exist.
 */
Sample* Device_params_get_sample(Device_params* params, const char* key);


/**
 * Retrieves Sample parameters from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonsh".
 *
 * \return   The Sample map, or \c NULL if \a key doesn't exist.
 */
Sample_params* Device_params_get_sample_params(Device_params* params,
                                               const char* key);


/**
 * Retrieves a Sample map from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonsm".
 *
 * \return   The Sample map, or \c NULL if \a key doesn't exist.
 */
Sample_map* Device_params_get_sample_map(Device_params* params,
                                         const char* key);


/**
 * Destroys existing Device parameters.
 *
 * \param params   The Device parameters, or \c NULL.
 */
void del_Device_params(Device_params* params);


#endif // K_DEVICE_PARAMS_H


