

#ifndef K_RELTIME_H
#define K_RELTIME_H


#include <stdint.h>


/**
 * This specifies how many parts one beat is. It is divisible by, among
 * others, powers of 2 up to 2^7, powers of 3 up to 3^4, and all positive
 * integers up to and including 18.
 */
#define RELTIME_FULL_PART (882161280L)


/**
 * This is the internal representation of relative time. Memory for this
 * object is expected to be initialised either automatically or as part of
 * another object.
 */
typedef struct Reltime
{
	int64_t beats;   ///< The number of beats.
	int32_t part;    ///< Part of a beat.
} Reltime;


/**
 * Intialises a Reltime object with time 0.
 *
 * \param r   The Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a d.
 */
Reltime* Reltime_init(Reltime* r);


/**
 * Compares two Reltime objects.
 *
 * \param r1   The first Reltime object -- must not be \c NULL.
 * \param r2   The second Reltime object -- must not be \c NULL.
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
 * \param part    The size of the remaining part -- must be >= \c 0 and
 *                < \c RELTIME_FULL_PART.
 *
 * \return   The parameter \a r.
 */
Reltime* Reltime_set(Reltime* r, int64_t beats, int32_t part);


/**
 * Computes the sum of two Reltime objects.
 *
 * \param result   The result Reltime object -- must not be \c NULL. This may
 *                 be the same as \a r1 and/or \a r2.
 * \param r1       The first Reltime object -- must not be \c NULL.
 * \param r2       The second Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a result.
 */
Reltime* Reltime_add(Reltime* result, const Reltime* r1, const Reltime* r2);


/**
 * Computes the difference between two Reltime objects.
 *
 * \param result   The result Reltime object -- must not be \c NULL. This may
 *                 be the same as \a r1 and/or \a r2.
 * \param r1       The subtracted Reltime object -- must not be \c NULL.
 * \param r2       The subtractor Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a result.
 */
Reltime* Reltime_sub(Reltime* result, const Reltime* r1, const Reltime* r2);


/**
 * Makes a copy of a Reltime object.
 *
 * \param dest   The destination Reltime object -- must not be \c NULL.
 * \param src    The source Reltime object -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Reltime* Reltime_copy(Reltime* dest, const Reltime* src);


/**
 * Converts the time represented by a Reltime object into frames.
 *
 * \param r       The Reltime object -- must not be \c NULL. Also, the time
 *                must be >= \c 0.
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


