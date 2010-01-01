

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


#ifndef K_INSTRUMENT_H
#define K_INSTRUMENT_H


#include <stdbool.h>

#include <Instrument_params.h>
#include <Generator.h>
#include <kunquat/frame.h>
#include <Event_queue.h>
#include <Voice_state.h>
#include <Scale.h>
#include <Envelope.h>
#include <kunquat/limits.h>


typedef struct Instrument Instrument;


#define INS_DEFAULT_FORCE (0)
#define INS_DEFAULT_FORCE_VAR (0)
#define INS_DEFAULT_SCALE_INDEX (-1)


/**
 * Creates a new Instrument.
 *
 * \param bufs            The global mixing buffers -- must not be \c NULL and
 *                        must contain at least \a buf_count buffers.
 * \param vbufs           The Voice mixing buffers -- must not be \c NULL and
 *                        must contain at least \a buf_count buffers.
 * \param vbufs2          The auxiliary Voice mixing buffers -- must not be \c NULL and
 *                        must contain at least \a buf_count buffers.
 * \param buf_count       The number of mixing buffers -- must be > \c 0.
 * \param buf_len         The length of a mixing buffer -- must be > \c 0.
 * \param scales          The Scales of the Song -- must not be \c NULL.
 * \param default_scale   The default Scale -- must not be \c NULL. Also,
 *                        *default_scales must be an element of \a scales.
 * \param events          The maximum number of events per tick -- must be > \c 0.
 *
 * \return   The new Instrument if successful, or \c NULL if memory allocation
 *           failed.
 */
Instrument* new_Instrument(kqt_frame** bufs,
                           kqt_frame** vbufs,
                           kqt_frame** vbufs2,
                           int buf_count,
                           uint32_t buf_len,
                           Scale** scales,
                           Scale*** default_scale,
                           uint8_t events);


/**
 * Parses an Instrument header from a textual description.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_parse_header(Instrument* ins, char* str, Read_state* state);


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
 * Gets common Generator parameters of a Generator in the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 *
 * \return   The parameters. Note that this is not a valid Generator.
 */
Generator* Instrument_get_common_gen_params(Instrument* ins, int index);


/**
 * Gets the number of Generators used by the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The number of Generators.
 */
// int Instrument_get_gen_count(Instrument* ins);


/**
 * Sets a Generator of the Instrument.
 *
 * If a Generator already exists at the specified index, it will be removed.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 * \param gen     The Generator -- must not be \c NULL.
 */
void Instrument_set_gen(Instrument* ins,
                        int index,
                        Generator* gen);


/**
 * Gets a Generator of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
Generator* Instrument_get_gen(Instrument* ins, int index);


/**
 * Sets a Generator of the Instrument based on the Generator type.
 *
 * Only Parse manager should use this function. It does not change the
 * effective Generator unless it has the same type as the new Generator.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 * \param gen     The Generator -- must not be \c NULL.
 */
void Instrument_set_gen_of_type(Instrument* ins,
                                int index,
                                Generator* gen);


/**
 * Gets a Generator of the Instrument based on a Generator type.
 *
 * Only Parse manager should use this function. The Generator returned is
 * not necessarily the active one.
 *
 * \param ins        The Instrument -- must not be \c NULL.
 * \param index      The index of the Generator -- must be >= \c 0 and
 *                   < \c KQT_GENERATORS_MAX.
 * \param gen_type   The Generator type -- must be a valid type.
 *
 * \return   The Generator if one exists, otherwise \c NULL.
 */
Generator* Instrument_get_gen_of_type(Instrument* ins,
                                      int index,
                                      Gen_type type);


/**
 * Removes a Generator of the Instrument.
 *
 * The Generators located at greater indices will be shifted backward in the
 * table. If the target Generator doesn't exist, the Instrument won't be
 * modified.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 */
void Instrument_del_gen(Instrument* ins, int index);


/**
 * Sets the active Scale of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Scale -- must be >= \c 0 and
 *                < \c KQT_SCALES_MAX or \c -1 (default).
 */
void Instrument_set_scale(Instrument* ins, int index);


/**
 * Adds a new Event into the Instrument event queue.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The position of the Event.
 *
 * \return   \c true if successful, or \c false if the Event queue is full.
 */
bool Instrument_add_event(Instrument* ins, Event* event, uint32_t pos);


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


