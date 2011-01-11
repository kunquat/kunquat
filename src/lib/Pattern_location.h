

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


#ifndef K_PATTERN_LOCATION_H
#define K_PATTERN_LOCATION_H


typedef struct Pattern_location
{
    int subsong;
    int section;
} Pattern_location;


#define PATTERN_LOCATION_AUTO (&(Pattern_location){ .subsong = 0, \
                                                    .section = 0 })


/**
 * Creates a new Pattern location.
 *
 * \param subsong   The subsong number -- must be >= \c 0 and
 *                  < \c KQT_SUBSONGS_MAX.
 * \param section   The section number -- must be >= \c 0 and
 *                  < \c KQT_SECTIONS_MAX.
 *
 * \return   The new Pattern location if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern_location* new_Pattern_location(int subsong, int section);


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


