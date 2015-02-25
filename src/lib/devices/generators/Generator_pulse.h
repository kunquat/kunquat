

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


#ifndef K_GENERATOR_PULSE_H
#define K_GENERATOR_PULSE_H


#include <devices/Device_impl.h>
#include <devices/Generator.h>


/**
 * Create a new Pulse Generator.
 *
 * \return   The new Pulse Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_pulse(Generator* gen);


/**
 * Return Pulse Generator property information.
 *
 * \param gen             The Pulse Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Pulse Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Generator_pulse_property(const Generator* gen, const char* property_type);


#endif // K_GENERATOR_PULSE_H


