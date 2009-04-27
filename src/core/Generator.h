

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


/**
 * Generator is an object used for creating sound based on a specific sound
 * synthesising method.
 *
 * The Generator class does not contain a constructor -- only Generators of
 * a specific type can be created.
 */
typedef struct Generator
{
    Gen_type type;
    void (*init_state)(Voice_state*);
    void (*destroy)(struct Generator*);
    void (*mix)(struct Generator*, Voice_state*, uint32_t, uint32_t, uint32_t);
    Instrument_params* ins_params;
} Generator;


/**
 * Returns the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
Gen_type Generator_get_type(Generator* gen);


/**
 * Handles a given note as appropriate for the Generator.
 *
 * \param gen      The Generator -- must not be \c NULL.
 * \param states   The array of Voice states -- must not be \c NULL.
 * \param note     The note number -- must be >= \c 0 and
 *                 < \c NOTE_TABLE_NOTES.
 * \param mod      The note modifier -- must be < \c NOTE_TABLE_NOTE_MODS.
 *                 Negative value means that no modifier will be applied.
 * \param octave   The octave -- must be >= \c NOTE_TABLE_OCTAVE_FIRST
 *                 and <= \c NOTE_TABLE_OCTAVE_LAST.
 */
void Generator_process_note(Generator* ins,
        Voice_state* states,
        int note,
        int mod,
        int octave);


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


