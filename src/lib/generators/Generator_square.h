

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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
    double pulse_width;
} Generator_square;


/**
 * Creates a new Square Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Square Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_square(Instrument_params* ins_params);


/**
 * Tells whether the given subkey is part of the Square Generator specification.
 *
 * \param subkey   The subkey. This is the part after "generator_XX/".
 *
 * \return   \c true if and only if \a subkey is part of the specification.
 */
bool Generator_square_has_subkey(const char* subkey);


/**
 * Parses data associated with a Square Generator.
 *
 * \param gen      The Generator -- must be a valid Square Generator.
 * \param subkey   The subkey. This is the part after "generator_XX/".
 * \param data     The data -- must not be \c NULL unless \a length is 0.
 * \param length   The length of the data -- must be >= \c 0.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. The Read state error
 *           will _not_ be set if memory allocation failed.
 */
bool Generator_square_parse(Generator* gen,
                            const char* subkey,
                            void* data,
                            long length,
                            Read_state* state);


uint32_t Generator_square_mix(Generator* gen,
                              Voice_state* state,
                              uint32_t nframes,
                              uint32_t offset,
                              uint32_t freq,
                              double tempo,
                              int buf_count,
                              kqt_frame** bufs);


/**
 * Destroys an existing Square Generator.
 *
 * \param gen   The Square Generator -- must not be \c NULL.
 */
void del_Generator_square(Generator* gen);


#endif // K_GENERATOR_SQUARE_H


