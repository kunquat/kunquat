

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


#ifndef K_GENERATOR_PARAMS_H
#define K_GENERATOR_PARAMS_H


#include <stdbool.h>
#include <stdint.h>

#include <File_base.h>
#include <Generator_field.h>
#include <Real.h>
#include <Reltime.h>


typedef struct Generator_params Generator_params;


/**
 * Tells whether a given key is a Generator parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a Generator parameter key,
 *           otherwise \c false.
 */
bool key_is_generator_param(const char* key);


/**
 * Tells whether a given key is a real-time modifiable Generator parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a real-time modifiable Generator parameter
 *           key, otherwise \c false.
 */
bool key_is_real_time_generator_param(const char* key);


/**
 * Tells whether a given key is a text-mode Generator parameter key.
 *
 * The key is assumed to originate from Parse manager, in which case it is a
 * valid key.
 *
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if \a key is a text-mode Generator parameter key,
 *           otherwise \c false.
 */
bool key_is_text_generator_param(const char* key);


/**
 * Creates new Generator parameters.
 *
 * \return   The new Generator parameters if successful, otherwise \c NULL.
 */
Generator_params* new_Generator_params(void);


/**
 * Allocates memory for a key.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Generator_params_set_key(Generator_params* params, const char* key);


/**
 * Parses the Generator Event list.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param str      The textual description.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will _not_ be
 *           modified if memory allocation failed.
 */
bool Generator_params_parse_events(Generator_params* params,
                                   char* str,
                                   Read_state* state);


/**
 * Parses a Generator parameter value.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
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
bool Generator_params_parse_value(Generator_params* params,
                                  const char* key,
                                  void* data,
                                  long length,
                                  Read_state* state);


/**
 * Modifies an existing Generator parameter value.
 *
 * This function is used during playback.
 * Note that this function does not support sample files.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ directory.
 * \param str      The new value as a string.
 *
 * \return   \c true if the value was modified, otherwise \c false.
 */
bool Generator_params_modify_value(Generator_params* params,
                                   const char* key,
                                   char* str);


/**
 * Retrieves a reference to a boolean value from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonb".
 *
 * \return   The boolean value, or \c NULL if \a key doesn't exist.
 */
bool* Generator_params_get_bool(Generator_params* params, const char* key);


/**
 * Retrieves a reference to an integer value from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsoni".
 *
 * \return   The integer value, or \c NULL if \a key doesn't exist.
 */
int64_t* Generator_params_get_int(Generator_params* params, const char* key);


/**
 * Retrieves a reference to a floating point value from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonf".
 *
 * \return   The floating point value, or \c NULL if \a key doesn't exist.
 */
double* Generator_params_get_float(Generator_params* params, const char* key);


/**
 * Retrieves a Real value from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonr".
 *
 * \return   The Real value, or \c 1/1 if \a key doesn't exist.
 */
Real* Generator_params_get_real(Generator_params* params, const char* key);


/**
 * Retrieves a Reltime value from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsont".
 *
 * \return   The Reltime value, or \c [0, 0] if \a key doesn't exist.
 */
Reltime* Generator_params_get_reltime(Generator_params* params, const char* key);


/**
 * Retrieves a Sample from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".wv".
 *
 * \return   The Sample, or \c NULL if \a key doesn't exist.
 */
//Sample* Generator_params_get_sample(Generator_params* params, const char* key);


/**
 * Retrieves Sample parameters from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonsh".
 *
 * \return   The Sample map, or \c NULL if \a key doesn't exist.
 */
//Sample_params* Generator_params_get_sample_params(Generator_params* params,
//                                                  const char* key);


/**
 * Retrieves a Sample map from Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey starting after the
 *                 c/ or i/ directory and must have the suffix ".jsonsm".
 *
 * \return   The Sample map, or \c NULL if \a key doesn't exist.
 */
Sample_map* Generator_params_get_sample_map(Generator_params* params,
                                            const char* key);


/**
 * Destroys existing Generator parameters.
 *
 * \param params   The Generator parameters -- must not be \c NULL.
 */
void del_Generator_params(Generator_params* params);


#endif // K_GENERATOR_PARAMS_H


