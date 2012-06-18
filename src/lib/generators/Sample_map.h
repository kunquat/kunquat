

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SAMPLE_MAP_H
#define K_SAMPLE_MAP_H


#include <stdint.h>

#include <AAtree.h>
#include <File_base.h>
#include <Random.h>
#include <Sample_entry.h>


#define SAMPLE_MAP_RANDOMS_MAX (8)


typedef struct Sample_map Sample_map;


/**
 * Creates a new Sample map from a string.
 *
 * \param str     The textual description, or \c NULL.
 * \param state   The Read state -- must not be \c NULL unless \a str is
 *                also \c NULL.
 *
 * \return   The new Sample map if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Sample_map* new_Sample_map_from_string(char* str, Read_state* state);


/**
 * Adds a Sample entry into a Sample map.
 *
 * This function is for Generators that create their own Sample maps.
 *
 * \param map     The Sample map -- must not be \c NULL.
 * \param cents   The pitch in cents -- must be finite.
 * \param force   The force in decibels -- must be finite.
 * \param entry   The Sample entry -- must not be \c NULL. The function
 *                ignores the ref_freq value in the supplied entry and
 *                calculates it from \a cents. The Sample map does _not_
 *                assume ownership of \a entry so it can be automatically
 *                allocated.
 *
 * \return   \c true if successful, or \c false if memory allocation failed
 *           or this mapping position (\a cents, \a force) already contains
 *           \c SAMPLE_MAP_RANDOMS_MAX entries.
 */
bool Sample_map_add_entry(Sample_map* map,
                          double cents,
                          double force,
                          Sample_entry* entry);


/**
 * Gets a Sample entry from the Sample map.
 *
 * \param map      The Sample map -- must not be \c NULL.
 * \param cents    The pitch in cents -- must be finite.
 * \param force    The force in dB -- must be finite or -INFINITY.
 * \param random   The Random source -- must not be \c NULL.
 *
 * \return   The closest Sample entry, or \c NULL if the map is empty.
 */
const Sample_entry* Sample_map_get_entry(Sample_map* map,
                                         double cents,
                                         double force,
                                         Random* random);


/**
 * Destroys an existing Sample map.
 *
 * \param map   The Sample map, or \c NULL.
 */
void del_Sample_map(Sample_map* map);


#endif // K_SAMPLE_MAP_H


