

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


#define MSG_LEN_MAX 55


/**
 * Calculates the MD5 sum of a given string.
 *
 * \param str     The string -- must not be \c NULL or longer than
 *                \c MSG_LEN_MAX bytes.
 * \param lower   The storage location for the least significant half of the
 *                digest -- must not be \c NULL.
 * \param upper   The storage location for the most significant half of the
 *                digest -- must not be \c NULL.
 */
void md5(char* str, uint64_t* lower, uint64_t* upper);


#endif // K_MD5_H


