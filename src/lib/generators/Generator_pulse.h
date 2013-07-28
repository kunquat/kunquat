

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


#ifndef K_GENERATOR_PULSE_H
#define K_GENERATOR_PULSE_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_pulse
{
    Generator parent;
    double pulse_width;
} Generator_pulse;


/**
 * Creates a new Pulse Generator.
 *
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new Pulse Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_pulse(uint32_t buffer_size,
                               uint32_t mix_rate);


/**
 * Returns Pulse Generator property information.
 *
 * \param gen             The Pulse Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Pulse Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
char* Generator_pulse_property(Generator* gen, const char* property_type);


#endif // K_GENERATOR_PULSE_H


