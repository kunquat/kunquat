

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


#ifndef K_INSTRUMENT_H
#define K_INSTRUMENT_H


#include <wchar.h>
#include <stdbool.h>

#include <Instrument_params.h>
#include <Generator.h>
#include <frame_t.h>
#include <Event_queue.h>
#include <Voice_state.h>
#include <Note_table.h>
#include <Envelope.h>
#include <Song_limits.h>


typedef struct Instrument
{
    wchar_t name[INS_NAME_MAX]; ///< The name of the Instrument.
    Event_queue* events;        ///< Instrument event queue (esp. pedal events go here).

    double default_force;       ///< Default force.
    double force_variation;     ///< Force variation.

    Note_table** note_tables;   ///< The Note tables of the Song.
    Note_table** default_notes; ///< The default Note table of the Song.
    int notes_index;            ///< The index of the Note table used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Generators need.

    int gen_count;                   ///< Number of Generators.
    Generator* gens[GENERATORS_MAX]; ///< Generators.
} Instrument;


/**
 * Creates a new Instrument.
 *
 * \param bufs            The global mixing buffers -- must not be \c NULL.
 *                        Additionally, bufs[0] and bufs[1] must not be \c NULL.
 * \param vbufs           The Voice mixing buffers -- must not be \c NULL.
 *                        Additionally, vbufs[0] and vbufs[1] must not be \c NULL.
 * \param buf_count       The number of mixing buffers -- must be > \c 0.
 * \param buf_len         The length of a mixing buffer -- must be > \c 0.
 * \param note_tables     The Note tables of the Song -- must not be \c NULL.
 * \param default_notes   The default note table -- must not be \c NULL. Also,
 *                        *default_notes must be an element of \a note_tables.
 * \param events          The maximum number of events per tick -- must be > \c 0.
 *
 * \return   The new Instrument if successful, or \c NULL if memory allocation
 *           failed.
 */
Instrument* new_Instrument(frame_t** bufs,
                           frame_t** vbufs,
                           int buf_count,
                           uint32_t buf_len,
                           Note_table** note_tables,
                           Note_table** default_notes,
                           uint8_t events);


/**
 * Reads an Instrument from a File tree.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_read(Instrument* ins, File_tree* tree, Read_state* state);


/**
 * Gets the Instrument parameters of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Instrument parameters.
 */
Instrument_params* Instrument_get_params(Instrument* ins);


/**
 * Gets the number of Generators used by the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The number of Generators.
 */
int Instrument_get_gen_count(Instrument* ins);


/**
 * Sets a Generator of the Instrument.
 *
 * If a Generator already exists at the specified index, it will be removed.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c GENERATORS_MAX.
 * \param gen     The Generator -- must not be \c NULL.
 *
 * \return   The actual index of the Generator. This is less than or equal to
 *           \a index.
 */
int Instrument_set_gen(Instrument* ins,
        int index,
        Generator* gen);


/**
 * Gets a Generator of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c GENERATORS_MAX.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
Generator* Instrument_get_gen(Instrument* ins,
        int index);


/**
 * Removes a Generator of the Instrument.
 *
 * The Generators located at greater indices will be shifted backward in the
 * table. If the target Generator doesn't exist, the Instrument won't be
 * modified.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c GENERATORS_MAX.
 */
void Instrument_del_gen(Instrument* ins, int index);


/**
 * Sets the name of the Instrument.
 *
 * \param ins    The Instrument -- must not be \c NULL.
 * \param name   The name -- must not be \c NULL.
 */
void Instrument_set_name(Instrument* ins, wchar_t* name);


/**
 * Gets the name of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The name.
 */
wchar_t* Instrument_get_name(Instrument* ins);


/**
 * Sets the active Note table of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Note table -- must be >= \c 0 and
 *                < \c NOTE_TABLES_MAX or \c -1 (default).
 */
void Instrument_set_note_table(Instrument* ins, int index);


/**
 * Mixes the Instrument.
 *
 * \param ins       The Instrument -- must not be \c NULL.
 * \param states    The array of Voice states -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed -- must not be greater
 *                  than the mixing buffer size.
 * \param offset    The starting frame offset (\a nframes - \a offset are
 *                  actually mixed).
 * \param freq      The mixing frequency -- must be > \c 0.
 */
void Instrument_mix(Instrument* ins,
        Voice_state* states,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq);


/**
 * Destroys an existing Instrument.
 *
 * \param   The Instrument -- must not be \c NULL.
 */
void del_Instrument(Instrument* ins);


#endif // K_INSTRUMENT_H


