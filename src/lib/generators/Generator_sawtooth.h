

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
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 * \param gen_params   The Generator parameters -- must not be \c NULL.
 *
 * \return   The new Sawtooth Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_sawtooth(Instrument_params* ins_params,
                                  Generator_params* gen_params);


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


