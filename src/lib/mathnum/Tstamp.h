

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_TSTAMP_H
#define KQT_TSTAMP_H


#include <decl.h>
#include <kunquat/limits.h>

#include <inttypes.h>
#include <stdint.h>


/**
 * This is the internal representation of relative time. Memory for this
 * object is expected to be initialised either automatically or as part of
 * another object.
 */
struct Tstamp
{
    int64_t beats; /// The number of beats.
    int32_t rem; /// Remainder of a beat -- always >= \c 0 and < \c KQT_TSTAMP_BEAT.
};


/**
 * A new instance of an uninitialised Tstamp object with automatic storage
 * allocation.
 * Useful for passing as a parameter to an initialiser.
 */
#define TSTAMP_AUTO (&(Tstamp){ .beats = 0, .rem = 0 })


/**
 * Helpers for printf family of functions.
 */
#define PRIts "(%" PRId64 ", %" PRId32 ")"
#define PRIVALts(ts) (ts).beats, (ts).rem


/**
 * Intialise a Tstamp with time 0.
 *
 * \param ts   The Tstamp -- must not be \c NULL.
 *
 * \return   The parameter \a ts.
 */
Tstamp* Tstamp_init(Tstamp* ts);


/**
 * Compare two Tstamp objects.
 *
 * \param ts1   The first Tstamp object -- must be a valid Tstamp.
 * \param ts2   The second Tstamp object -- must be a valid Tstamp.
 *
 * \return   An integer less than, equal to or greater than zero if \a ts1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a ts2.
 */
int Tstamp_cmp(const Tstamp* ts1, const Tstamp* ts2);


/**
 * Set the parameters of a given Tstamp object.
 *
 * \param ts      The Tstamp object -- must not be \c NULL.
 * \param beats   The number of beats.
 * \param rem     The size of the remainder -- must be >= \c 0 and
 *                < \c KQT_TSTAMP_BEAT.
 *
 * \return   The parameter \a ts.
 */
Tstamp* Tstamp_set(Tstamp* ts, int64_t beats, int32_t rem);


/**
 * Get the beat count of a Tstamp.
 *
 * \param ts   The Tstamp -- must not be \c NULL.
 *
 * \return   The beat count.
 */
int64_t Tstamp_get_beats(const Tstamp* ts);


/**
 * Get the remainder part of a Tstamp.
 *
 * \param ts   The Tstamp -- must not be \c NULL.
 *
 * \return   The remainder part.
 */
int32_t Tstamp_get_rem(const Tstamp* ts);


/**
 * Compute the sum of two Tstamps.
 *
 * If the beat count overflows during the calculation, the behaviour is
 * dependent on the underlying C implementation.
 *
 * \param result   The result Tstamp -- must not be \c NULL. This may
 *                 be the same as \a ts1 and/or \a ts2.
 * \param ts1      The first Tstamp object -- must be a valid Tstamp.
 * \param ts2      The second Tstamp object -- must be a valid Tstamp.
 *
 * \return   The parameter \a result.
 */
Tstamp* Tstamp_add(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2);


/**
 * Combined Tstamp addition and assignment.
 */
#define Tstamp_adda(ts1, ts2) Tstamp_add((ts1), (ts1), (ts2))


/**
 * Compute the difference between two Tstamps.
 *
 * If the beat count overflows during the calculation, the behaviour is
 * dependent on the underlying C implementation.
 *
 * \param result   The result Tstamp -- must not be \c NULL. This may
 *                 be the same as \a ts1 and/or \a ts2.
 * \param ts1      The subtracted Tstamp -- must be a valid Tstamp.
 * \param ts2      The subtractor Tstamp -- must be a valid Tstamp.
 *
 * \return   The parameter \a result.
 */
Tstamp* Tstamp_sub(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2);


/**
 * Combined Tstamp subtraction and assignment.
 */
#define Tstamp_suba(ts1, ts2) Tstamp_sub((ts1), (ts1), (ts2))


/**
 * Make a copy of a Tstamp.
 *
 * \param dest   The destination Tstamp -- must not be \c NULL.
 * \param src    The source Tstamp -- must be a valid Tstamp.
 *
 * \return   The parameter \a dest.
 */
Tstamp* Tstamp_copy(Tstamp* dest, const Tstamp* src);


/**
 * Return the minimum of two Tstamps.
 *
 * \param result   The result Tstamp -- must not be \c NULL. This may
 *                 be the same as \a ts1 and/or \a ts2.
 * \param ts1      The first Tstamp -- must be valid.
 * \param ts2      The second Tstamp -- must be valid.
 *
 * \return   The parameter \a result.
 */
Tstamp* Tstamp_min(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2);


/**
 * Combined Tstamp minimum and assignment.
 */
#define Tstamp_mina(ts1, ts2) Tstamp_min((ts1), (ts1), (ts2))


/**
 * Convert the time represented by a Tstamp into frames.
 *
 * \param ts      The Tstamp -- must be valid. Also, the
 *                time must be >= \c 0.
 * \param tempo   The tempo -- must be > \c 0.
 * \param rate    The audio rate -- must be > \c 0.
 *
 * \return   The number of frames.
 */
double Tstamp_toframes(const Tstamp* ts, double tempo, uint32_t rate);


/**
 * Convert the time represented by frames into a Tstamp object.
 *
 * \param ts       The Tstamp -- must not be \c NULL.
 * \param frames   The number of frames.
 * \param tempo    The tempo -- must be > \c 0.
 * \param rate     The audio rate -- must be > \c 0.
 *
 * \return   The parameter \a ts.
 */
Tstamp* Tstamp_fromframes(Tstamp* ts, uint32_t frames, double tempo, uint32_t rate);


#endif // KQT_TSTAMP_H


