

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


#ifndef K_SAMPLE_MAP_H
#define K_SAMPLE_MAP_H


#include <stdint.h>

#include <AAtree.h>
#include <File_base.h>
#include <Random.h>


#define SAMPLE_MAP_RANDOMS_MAX (8)


typedef struct Sample_map Sample_map;


typedef struct Sample_entry
{
    double ref_freq;  ///< The reference frequency in the mapping.
    double freq;      ///< The playback frequency of this sample in the reference frequency.
    double vol_scale;
    uint16_t sample;
} Sample_entry;


/**
 * Creates a new Sample map from a string.
 *
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Sample map if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Sample_map* new_Sample_map_from_string(char* str, Read_state* state);


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
 * \param map   The Sample map -- must not be \c NULL.
 */
void del_Sample_map(Sample_map* map);


#endif // K_SAMPLE_MAP_H


