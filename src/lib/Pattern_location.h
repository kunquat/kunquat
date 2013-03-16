

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PATTERN_LOCATION_H
#define K_PATTERN_LOCATION_H


#include <Pat_inst_ref.h>


typedef struct Pattern_location
{
    int song;
    Pat_inst_ref piref;
} Pattern_location;


#define PATTERN_LOCATION_AUTO \
    (&(Pattern_location){ .song = 0, .piref = *PAT_INST_REF_AUTO })


/**
 * Creates a new Pattern location.
 *
 * \param song    The subsong number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 * \param piref   The Pattern instance reference -- must be valid.
 *
 * \return   The new Pattern location if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern_location* new_Pattern_location(int song, Pat_inst_ref* piref);


/**
 * Compares two Pattern locations.
 *
 * \param loc1   The first Pattern location -- must not be \c NULL.
 * \param loc2   The second Pattern location -- must not be \c NULL.
 *
 * \return   An integer less, equal to, or greater than zero if \a loc1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a loc2.
 */
int Pattern_location_cmp(const Pattern_location* loc1,
                         const Pattern_location* loc2);


#endif // K_PATTERN_LOCATION_H


