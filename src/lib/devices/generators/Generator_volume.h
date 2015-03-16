

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_VOLUME_H
#define K_GENERATOR_VOLUME_H


#include <stdint.h>

#include <devices/Device_impl.h>
#include <devices/Generator.h>


/**
 * Create a new volume Generator.
 *
 * \return   The new volume Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_volume(Generator* gen);


#endif // K_GENERATOR_VOLUME_H


