

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_SAWTOOTH_H
#define K_GENERATOR_SAWTOOTH_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_sawtooth
{
    Generator parent;
} Generator_sawtooth;


/**
 * Creates a new Sawtooth Generator.
 *
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new Sawtooth Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_sawtooth(uint32_t buffer_size,
                                  uint32_t mix_rate);


/**
 * Returns Sawtooth Generator property information.
 *
 * \param gen             The Sawtooth Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Sawtooth Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
char* Generator_sawtooth_property(Generator* gen, const char* property_type);


uint32_t Generator_sawtooth_mix(Generator* gen,
                                Voice_state* state,
                                uint32_t nframes,
                                uint32_t offset,
                                uint32_t freq,
                                double tempo);


/**
 * Destroys an existing Sawtooth Generator.
 *
 * \param gen   The Sawtooth Generator -- must not be \c NULL.
 */
void del_Generator_sawtooth(Generator* gen);


#endif // K_GENERATOR_SAWTOOTH_H


