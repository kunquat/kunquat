

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_PARAMS_H
#define KQT_DEVICE_PARAMS_H


#include <containers/AAtree.h>
#include <init/devices/Device_field.h>
#include <mathnum/Tstamp.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>


typedef struct Device_params Device_params;


typedef struct Device_params_iter
{
    AAiter impl_iter;
    AAiter config_iter;
    const char* next_impl_key;
    const char* next_config_key;
} Device_params_iter;


#define DEVICE_PARAMS_ITER_AUTO \
    (&(Device_params_iter){ { .tree = NULL }, { .tree = NULL }, NULL, NULL })


/**
 * Initialise a Device params iterator.
 *
 * \param iter      The Device params iterator -- must not be \c NULL.
 * \param dparams   The Device parameters -- must not be \c NULL.
 *
 * \return   The parameter \a iter.
 */
Device_params_iter* Device_params_iter_init(
        Device_params_iter* iter, const Device_params* dparams);


/**
 * Get the next Device parameter key from the iterator.
 *
 * \param iter   The Device params iterator -- must not be \c NULL.
 *
 * \return   The next key, or \c NULL if reached the end of the parameters.
 */
const char* Device_params_iter_get_next_key(Device_params_iter* iter);


/**
 * Tell whether a given key is a Device parameter key.
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
 * Tell whether a given key is a real-time modifiable Device parameter key.
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
 * Tell whether a given key is a text-mode Device parameter key.
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
 * Create new Device parameters.
 *
 * \return   The new Device parameters if successful, otherwise \c NULL.
 */
Device_params* new_Device_params(void);


/**
 * Allocate memory for a key.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
//bool Device_params_set_key(Device_params* params, const char* key);


/**
 * Mark a key to require explicit synchronisation on update.
 *
 * This is used for keys that are too slow to be updated in the Device
 * automatically (e.g. the update may take hundreds of milliseconds or more).
 * This is different from keys that cannot be updated in real time but still
 * update quickly in terms of user interaction smoothness.
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
//bool Device_params_set_slow_sync(Device_params* params, const char* key);


/**
 * Mark a key to no longer require explicit synchronisation on update.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 */
//void Device_params_clear_slow_sync(Device_params* params, const char* key);


/**
 * Find out whether any slow-sync parameters have changed.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 *
 * \return   \c true if explicit synchronisation is needed,
 *           otherwise \c false.
 */
//bool Device_params_need_sync(Device_params* params);


/**
 * Retrieve a slow-sync key that has changed.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 *
 * \return   One of the slow-sync keys, or \c NULL if no such keys are left.
 */
//const char* Device_params_get_slow_sync_key(Device_params* params);


/**
 * Set the Device parameters synchronised.
 *
 * A slow-syncing Device should call this after synchronisation.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 */
//void Device_params_synchronised(Device_params* params);


/**
 * Parse the Device Event list.
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
 * Parse a Device parameter value.
 *
 * \param params    The Device parameters -- must not be \c NULL.
 * \param key       The key -- must be a valid subkey with the i/ or c/ as
 *                  the first component.
 * \param version   The data version -- must be >= \c 0.
 * \param sr        The Streader of the data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Device_params_parse_value(
        Device_params* params, const char* key, int version, Streader* sr);


/**
 * Modify an existing Device parameter value.
 *
 * This function is used during playback.
 * Note that this function does not support sample files.
 * Also, the function will not modify keys that are marked to require explicit
 * synchronisation.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 * \param data     The new value -- must not be \c NULL.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
//bool Device_params_modify_value(Device_params* params,
//                                const char* key,
//                                void* data);


/**
 * Reset Device parameter modifications.
 *
 * This function resets modifications made using Device_params_modify_value.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 */
//void Device_params_reset(Device_params* params);


/**
 * Retrieve a reference to a boolean value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid boolean subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The boolean value, or \c NULL if \a key doesn't exist.
 */
const bool* Device_params_get_bool(const Device_params* params, const char* key);


/**
 * Retrieve a reference to an integer value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid integer subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The integer value, or \c NULL if \a key doesn't exist.
 */
const int64_t* Device_params_get_int(const Device_params* params, const char* key);


/**
 * Retrieve a reference to a floating point value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid floating point subkey starting
 *                 after the c/ or i/ directory.
 *
 * \return   The floating point value, or \c NULL if \a key doesn't exist.
 */
const double* Device_params_get_float(const Device_params* params, const char* key);


/**
 * Retrieve a Tstamp value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Tstamp subkey starting after the
 *                 c/ or i/ directory.
 *
 * \return   The Tstamp value, or \c NULL if \a key doesn't exist.
 */
const Tstamp* Device_params_get_tstamp(const Device_params* params, const char* key);


/**
 * Retrieve an Envelope value from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Envelope subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The Envelope value, or \c NULL if \a key doesn't exist.
 */
const Envelope* Device_params_get_envelope(const Device_params* params, const char* key);


/**
 * Retrieve a Sample from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid sample subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The Sample, or \c NULL if \a key doesn't exist.
 */
const Sample* Device_params_get_sample(const Device_params* params, const char* key);


/**
 * Retrieve Sample parameters from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Sample parameters subkey
 *                 starting after the c/ or i/ directory.
 * \return   The Sample parameters, or \c NULL if \a key doesn't exist.
 */
const Sample_params* Device_params_get_sample_params(
        const Device_params* params, const char* key);


/**
 * Retrieve a Note map from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Note map subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The Note map, or \c NULL if \a key doesn't exist.
 */
const Note_map* Device_params_get_note_map(const Device_params* params, const char* key);


/**
 * Retrieve a Hit map from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Hit map subkey starting after
 *                 the c/ or i/ directory.
 *
 * \return   The Hit map, or \c NULL if \a key doesn't exist.
 */
const Hit_map* Device_params_get_hit_map(const Device_params* params, const char* key);


/**
 * Retrieve a Number list from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Number list subkey starting
 *                 after the c/ or i/ directory.
 *
 * \return   The Number list, or \c NULL if \a key doesn't exist.
 */
const Num_list* Device_params_get_num_list(const Device_params* params, const char* key);


/**
 * Retrieve PADsynth parameters from Device parameters.
 *
 * \param params   The Device parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid Number list subkey starting
 *                 after the c/ or i/ directory.
 *
 * \return   The PADsynth parameters, or \c NULL if \a key doesn't exist.
 */
const Padsynth_params* Device_params_get_padsynth_params(
        const Device_params* params, const char* key);


/**
 * Destroy existing Device parameters.
 *
 * \param params   The Device parameters, or \c NULL.
 */
void del_Device_params(Device_params* params);


#endif // KQT_DEVICE_PARAMS_H


