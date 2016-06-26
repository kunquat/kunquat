

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


#ifndef KQT_HIT_MAP_H
#define KQT_HIT_MAP_H


#include <init/devices/param_types/Sample_entry.h>
#include <mathnum/Random.h>
#include <string/Streader.h>

#include <stdlib.h>


#define HIT_MAP_RANDOMS_MAX 8


typedef struct Hit_map Hit_map;


/**
 * Create a new Hit map from a string.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Hit map if successful, otherwise \c NULL.
 */
Hit_map* new_Hit_map_from_string(Streader* sr);


/**
 * Get a Sample entry from the Hit map.
 *
 * \param map         The Hit map -- must not be \c NULL.
 * \param hit_index   The hit index -- must be >= \c 0 and < \c KQT_HITS_MAX.
 * \param force       The force in dB -- must be finite or -INFINITY.
 * \param random      The Random source -- must not be \c NULL.
 *
 * \return   The closest Sample entry, or \c NULL if the map does not contain
 *           entries for the given hit index.
 */
const Sample_entry* Hit_map_get_entry(
        const Hit_map* map, int hit_index, double force, Random* random);


/**
 * Destroy an existing Hit map.
 *
 * \param map   The Hit map, or \c NULL.
 */
void del_Hit_map(Hit_map* map);


#endif // KQT_HIT_MAP_H


