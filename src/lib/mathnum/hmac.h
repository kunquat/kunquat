

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_HMAC_H
#define KQT_HMAC_H


#include <stdint.h>
#include <stdlib.h>


/**
 * Calculate the HMAC of the given key and message.
 *
 * This implementation is restricted to 64-bit keys and string messages.
 *
 * \param key     The key.
 * \param msg     The message -- must not be \c NULL.
 * \param lower   The storage location for the least significant half of the
 *                digest -- must not be \c NULL.
 * \param upper   The storage location for the most significant half of the
 *                digest -- must not be \c NULL.
 */
void hmac_md5(uint64_t key, const char* msg, uint64_t* lower, uint64_t* upper);


#endif // KQT_HMAC_H


