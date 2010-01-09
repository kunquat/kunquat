

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_RELTIME_H
#define K_RELTIME_H


#include <stdint.h>

#include <String_buffer.h>
#include <kunquat/limits.h>


/**
 * This is the internal representation of relative time. Memory for this
 * object is expected to be initialised either automatically or as part of
 * another object.
 */
typedef struct Reltime
{
    int64_t beats; /// The number of beats.
    int32_t rem; /// Remainder of a beat -- always >= \c 0 and < \c KQT_RELTIME_BEAT.
} Reltime;


/**
 * A new instance of an uninitialised Reltime object with automatic storage
 * allocation.
 * Useful for passing as a parameter to an initialiser.
 */
#define RELTIME_AUTO (&(Reltime){ .beats = 0, .rem = 0 })


/**
 * Intialises a Reltime object with time 0.
 *
 * \param r   The Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a d.
 */
Reltime* Reltime_init(Reltime* r);


/**
 * Serialises a Reltime.
 *
 * \param r    The Reltime object -- must not be \c NULL.
 * \param sb   The String buffer where the Reltime shall be written -- must
 *             not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Reltime_serialise(Reltime* r, String_buffer* sb);


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
int Reltime_cmp(const Reltime* r1, const Reltime* r2);


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
Reltime* Reltime_set(Reltime* r, int64_t beats, int32_t rem);


/**
 * Gets the beat count of a Reltime.
 *
 * \param r       The Reltime -- must not be \c NULL.
 *
 * \return   The beat count.
 */
int64_t Reltime_get_beats(const Reltime* r);


/**
 * Gets the remainder part of a Reltime.
 *
 * \param r       The Reltime -- must not be \c NULL.
 *
 * \return   The remainder part.
 */
int32_t Reltime_get_rem(const Reltime* r);


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
Reltime* Reltime_add(Reltime* result, const Reltime* r1, const Reltime* r2);


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
Reltime* Reltime_sub(Reltime* result, const Reltime* r1, const Reltime* r2);


/**
 * Makes a copy of a Reltime object.
 *
 * \param dest   The destination Reltime object -- must not be \c NULL.
 * \param src    The source Reltime object -- must be a valid Reltime.
 *
 * \return   The parameter \a dest.
 */
Reltime* Reltime_copy(Reltime* dest, const Reltime* src);


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
uint32_t Reltime_toframes(const Reltime* r,
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
Reltime* Reltime_fromframes(Reltime* r,
                            uint32_t frames,
                            double tempo,
                            uint32_t freq);


#endif // K_RELTIME_H


