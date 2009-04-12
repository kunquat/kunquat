

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_GENERATOR_H
#define K_GENERATOR_H


#include <Generator_type.h>
#include <Instrument_params.h>
#include <Voice_state.h>


typedef struct Generator
{
    Gen_type type;
    int (*init)(struct Generator*);
    void (*init_state)(Voice_state*);
    void (*uninit)(struct Generator*);
    void (*destroy)(struct Generator*);
    void (*mix)(struct Generator*, Voice_state*, uint32_t, uint32_t, uint32_t);
    Instrument_params* ins_params;
} Generator;


/**
 * Creates a new Generator.
 *
 * \param type   The type of the Generator -- must be a valid type.
 *
 * \return   The new Generator if successful, or \c NULL if memory allocation
 *           failed.
 */
bool Generator_init(Generator* gen, Gen_type type);


/**
 * Returns the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
Gen_type Generator_get_type(Generator* gen);


/**
 * Mixes the Generator.
 *
 * \param gen       The Generator -- must not be \c NULL.
 * \param state     The Voice state -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed -- must not be greater
 *                  than the mixing buffer size.
 * \param offset    The starting frame offset (\a nframes - \a offset are
 *                  actually mixed).
 * \param freq      The mixing frequency -- must be > \c 0.
 */
void Generator_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq);


/**
 * Uninitialises an existing Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 */
void del_Generator(Generator* gen);


#endif // K_GENERATOR_H


