

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_INPUT_MAP_H
#define K_INPUT_MAP_H


#include <stdint.h>
#include <stdlib.h>

#include <Bit_array.h>
#include <string/Streader.h>


/**
 * A map from input IDs to device table entries.
 */
typedef struct Input_map Input_map;


/**
 * Creates a new Input map.
 *
 * \param sr            The Streader of the JSON input -- must not be \c NULL.
 * \param num_inputs    Number of distinct input values -- must be > \c 0.
 * \param num_outputs   Number of distinct output values -- must be > \c 0.
 *
 * \return   The new Input map if successful, or \c NULL if failed.
 */
Input_map* new_Input_map(
        Streader* sr, int32_t num_inputs, int32_t num_outputs);


/**
 * Checks if the Input map uses only existent controls.
 *
 * \param im          The Input map -- must not be \c NULL.
 * \param existents   The table of existent statuses -- must not be \c NULL.
 *
 * \return   \c true if \a im is valid, otherwise \c false.
 */
bool Input_map_is_valid(const Input_map* im, const Bit_array* existents);


/**
 * Returns a device table index from the Input map.
 *
 * \param im         The Input map -- must not be \c NULL.
 * \param input_id   The input ID -- must be >= \c 0 and less than
 *                   \a num_inputs passed to the constructor.
 *
 * \return   The device table ID, or \c -1 if not found.
 */
int32_t Input_map_get_device_index(const Input_map* im, int32_t input_id);


/**
 * Destroys an Input map.
 *
 * \param im   The Input map, or \c NULL.
 */
void del_Input_map(Input_map* im);


#endif // K_INPUT_MAP_H


