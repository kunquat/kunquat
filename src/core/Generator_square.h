

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


#ifndef K_GENERATOR_SQUARE_H
#define K_GENERATOR_SQUARE_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_square
{
    Generator parent;
} Generator_square;


/**
 * Creates a new Square Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Square Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator_square* new_Generator_square(Instrument_params* ins_params);


uint32_t Generator_square_mix(Generator* gen,
                              Voice_state* state,
                              uint32_t nframes,
                              uint32_t offset,
                              uint32_t freq,
                              int buf_count,
                              frame_t** bufs);


/**
 * Destroys an existing Square Generator.
 *
 * \param gen   The Square Generator -- must not be \c NULL.
 */
void del_Generator_square(Generator* gen);


#endif // K_GENERATOR_SQUARE_H


