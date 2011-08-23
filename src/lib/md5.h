

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


#ifndef K_MD5_H
#define K_MD5_H


#include <stdint.h>


/**
 * Calculates the MD5 sum of a given byte sequence.
 *
 * \param seq     The sequence -- must not be \c NULL.
 * \param len     The length of \a seq -- must be >= \c 0.
 * \param lower   The storage location for the least significant half of the
 *                digest -- must not be \c NULL.
 * \param upper   The storage location for the most significant half of the
 *                digest -- must not be \c NULL.
 */
void md5(char* seq, int len, uint64_t* lower, uint64_t* upper);


/**
 * Calculates the MD5 sum of a given byte sequence with custom initial state.
 *
 * \param seq          The sequence -- must not be \c NULL.
 * \param len          The length of \a seq -- must be >= \c 0.
 * \param lower        The storage location for the least significant half of
 *                     the digest -- must not be \c NULL.
 * \param upper        The storage location for the most significant half of
 *                     the digest -- must not be \c NULL.
 * \param lower_init   The initial lower part of the state.
 * \param upper_init   The initial upper part of the state.
 */
void md5_with_state(char* seq, int len, uint64_t* lower, uint64_t* upper,
                    uint64_t lower_init, uint64_t upper_init);


/**
 * Calculates the MD5 sum of a given string.
 *
 * \param str     The string -- must not be \c NULL.
 * \param lower   The storage location for the least significant half of the
 *                digest -- must not be \c NULL.
 * \param upper   The storage location for the most significant half of the
 *                digest -- must not be \c NULL.
 */
void md5_str(char* str, uint64_t* lower, uint64_t* upper);


#endif // K_MD5_H


