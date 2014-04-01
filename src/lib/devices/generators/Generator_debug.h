

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <devices/Device_impl.h>
#include <Generator.h>


/**
 * Creates a new Debug Generator.
 *
 * The Debug Generator generates a narrow pulse wave (with one sample value 1,
 * the rest are 0.5) that lasts no more than 10 phase cycles. Note off lasts
 * no more than two phase cycles with all sample values negated.
 *
 * \return   The new Debug Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_debug(Generator* gen);


#endif // K_GENERATOR_DEBUG_H


