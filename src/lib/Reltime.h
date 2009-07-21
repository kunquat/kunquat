

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_RELTIME_H
#define K_RELTIME_H


#include <stdint.h>

#include <kunquat/limits.h>


/**
 * This is the internal representation of relative time. Memory for this
 * object is expected to be initialised either automatically or as part of
 * another object.
 */
typedef struct kqt_Reltime
{
    int64_t beats; /// The number of beats.
    int32_t rem; /// Remainder of a beat -- always >= \c 0 and < \c KQT_RELTIME_BEAT.
} kqt_Reltime;


/**
 * A new instance of an uninitialised Reltime object with automatic storage
 * allocation.
 * Useful for passing as a parameter to an initialiser.
 */
#define KQT_RELTIME_AUTO (&(kqt_Reltime){ .beats = 0 })


/**
 * Intialises a Reltime object with time 0.
 *
 * \param r   The Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a d.
 */
kqt_Reltime* kqt_Reltime_init(kqt_Reltime* r);


/**
 * Compares two Reltime objects.
 *
 * \param r1   The first Reltime object -- must be a valid Reltime.
 * \param r2   The second Reltime object -- must be a valid Reltime.
 *
 * \return   An integer less than, equal to or greater than zero if \a r1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a r2.
 */
int kqt_Reltime_cmp(const kqt_Reltime* r1, const kqt_Reltime* r2);


/**
 * Sets the parameters of a given Reltime object.
 *
 * \param r       The Reltime object -- must not be \c NULL.
 * \param beats   The number of beats.
 * \param rem     The size of the remaining part -- must be >= \c 0 and
 *                < \c RELTIME_BEAT.
 *
 * \return   The parameter \a r.
 */
kqt_Reltime* kqt_Reltime_set(kqt_Reltime* r, int64_t beats, int32_t rem);


/**
 * Gets the beat count of a Reltime.
 *
 * \param r       The Reltime -- must not be \c NULL.
 *
 * \return   The beat count.
 */
int64_t kqt_Reltime_get_beats(const kqt_Reltime* r);


/**
 * Gets the remainder part of a Reltime.
 *
 * \param r       The Reltime -- must not be \c NULL.
 *
 * \return   The remainder part.
 */
int32_t kqt_Reltime_get_rem(const kqt_Reltime* r);


/**
 * Computes the sum of two Reltime objects.
 *
 * If the beat count overflows during the calculation, the behaviour is
 * dependent on the underlying C implementation.
 *
 * \param result   The result Reltime object -- must not be \c NULL. This may
 *                 be the same as \a r1 and/or \a r2.
 * \param r1       The first Reltime object -- must be a valid Reltime.
 * \param r2       The second Reltime object -- must be a valid Reltime.
 *
 * \return   The parameter \a result.
 */
kqt_Reltime* kqt_Reltime_add(kqt_Reltime* result, const kqt_Reltime* r1, const kqt_Reltime* r2);


/**
 * Computes the difference between two Reltime objects.
 *
 * If the beat count overflows during the calculation, the behaviour is
 * dependent on the underlying C implementation.
 *
 * \param result   The result Reltime object -- must not be \c NULL. This may
 *                 be the same as \a r1 and/or \a r2.
 * \param r1       The subtracted Reltime object -- must be a valid Reltime.
 * \param r2       The subtractor Reltime object -- must be a valid Reltime.
 *
 * \return   The parameter \a result.
 */
kqt_Reltime* kqt_Reltime_sub(kqt_Reltime* result, const kqt_Reltime* r1, const kqt_Reltime* r2);


/**
 * Makes a copy of a Reltime object.
 *
 * \param dest   The destination Reltime object -- must not be \c NULL.
 * \param src    The source Reltime object -- must be a valid Reltime.
 *
 * \return   The parameter \a dest.
 */
kqt_Reltime* kqt_Reltime_copy(kqt_Reltime* dest, const kqt_Reltime* src);


/**
 * Converts the time represented by a Reltime object into frames.
 *
 * \param r       The Reltime object -- must be a valid Reltime. Also, the
 *                time must be >= \c 0.
 * \param tempo   The tempo -- must be > \c 0.
 * \param freq    The mixing frequency -- must be > \c 0.
 *
 * \return   The number of frames.
 */
uint32_t kqt_Reltime_toframes(const kqt_Reltime* r,
                              double tempo,
                              uint32_t freq);


/**
 * Converts the time represented by frames into a Reltime object.
 *
 * \param r        The Reltime object -- must not be \c NULL.
 * \param frames   The number of frames.
 * \param tempo    The tempo -- must be > \c 0.
 * \param freq     The mixing frequency -- must be > \c 0.
 *
 * \return   The parameter \a r.
 */
kqt_Reltime* kqt_Reltime_fromframes(kqt_Reltime* r,
                                    uint32_t frames,
                                    double tempo,
                                    uint32_t freq);


#endif // K_RELTIME_H


