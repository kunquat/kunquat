

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_DELAY_H
#define K_GENERATOR_DELAY_H


#include <stdint.h>

#include <devices/Device_impl.h>
#include <devices/Generator.h>


/**
 * Create a new delay Generator.
 *
 * This Generator implements a tapped delay line (TDL).
 *
 * \return   The new delay Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_delay(Generator* gen);


#endif // K_GENERATOR_DELAY_H


