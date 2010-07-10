

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


#ifndef K_GENERATOR_TRIANGLE_H
#define K_GENERATOR_TRIANGLE_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_triangle
{
    Generator parent;
} Generator_triangle;


/**
 * Creates a new Triangle Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 * \param gen_params   The Generator parameters -- must not be \c NULL.
 *
 * \return   The new Triangle Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_triangle(Instrument_params* ins_params,
                                  Generator_params* gen_params);


uint32_t Generator_triangle_mix(Generator* gen,
                                Voice_state* state,
                                uint32_t nframes,
                                uint32_t offset,
                                uint32_t freq,
                                double tempo);


/**
 * Destroys an existing Triangle Generator.
 *
 * \param gen   The Triangle Generator -- must not be \c NULL.
 */
void del_Generator_triangle(Generator* gen);


#endif // K_GENERATOR_TRIANGLE_H


