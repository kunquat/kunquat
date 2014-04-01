

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2013
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_NOISE_H
#define K_GENERATOR_NOISE_H


#include <Device_impl.h>
#include <Generator.h>


/**
 * Creates a new Noise Generator.
 *
 * \return   The new Noise Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_noise(Generator* gen);


/**
 * Returns Noise Generator property information.
 *
 * \param gen             The Noise Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Noise Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
char* Generator_noise_property(Generator* gen, const char* property_type);


#endif // K_GENERATOR_NOISE_H


