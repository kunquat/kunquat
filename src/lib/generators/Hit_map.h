

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


#ifndef K_HIT_MAP_H
#define K_HIT_MAP_H


#include <stdlib.h>

#include <File_base.h>
#include <Random.h>
#include <Sample_entry.h>


#define HIT_MAP_RANDOMS_MAX 8


typedef struct Hit_map Hit_map;


/**
 * Creates a new Hit map from a string.
 *
 * \param str     The textual description, or \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Hit map if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Hit_map* new_Hit_map_from_string(char* str, Read_state* state);


/**
 * Gets a Sample entry from the Hit map.
 *
 * \param map         The Hit map -- must not be \c NULL.
 * \param hit_index   The hit index -- must be >= \c 0 and < \c KQT_HITS_MAX.
 * \param force       The force in dB -- must be finite or -INFINITY.
 * \param random      The Random source -- must not be \c NULL.
 *
 * \return   The closest Sample entry, or \c NULL if the map does not contain
 *           entries for the given hit index.
 */
const Sample_entry* Hit_map_get_entry(Hit_map* map,
                                      int hit_index,
                                      double force,
                                      Random* random);


/**
 * Destroys an existing Hit map.
 *
 * \param map   The Hit map, or \c NULL.
 */
void del_Hit_map(Hit_map* map);


#endif // K_HIT_MAP_H


