

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_DEBUG_H
#define K_GENERATOR_DEBUG_H


#include <stdint.h>

#include <Device_impl.h>
#include <Generator.h>


/**
 * Creates a new Debug Generator.
 *
 * The Debug Generator generates a narrow pulse wave (with one sample value 1,
 * the rest are 0.5) that lasts no more than 10 phase cycles. Note off lasts
 * no more than two phase cycles with all sample values negated.
 *
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new Debug Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_debug(Generator* gen, uint32_t buffer_size, uint32_t mix_rate);


#endif // K_GENERATOR_DEBUG_H


