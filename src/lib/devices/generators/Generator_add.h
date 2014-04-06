

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_ADD_H
#define K_GENERATOR_ADD_H


#include <stdint.h>

#include <devices/Device_impl.h>
#include <devices/Generator.h>


/**
 * Create a new additive synthesis Generator.
 *
 * \return   The new additive synthesis Generator if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_impl* new_Generator_add(Generator* gen);


/**
 * Return additive Generator property information.
 *
 * \param gen             The additive Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The additive Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Generator_add_property(Generator* gen, const char* property_type);


#endif // K_GENERATOR_ADD_H


