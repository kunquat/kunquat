

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_INSTRUMENT_H
#define K_INSTRUMENT_H


#include <stdbool.h>

#include <Connections.h>
#include <Effect.h>
#include <Effect_table.h>
#include <Instrument_params.h>
#include <Gen_table.h>
#include <Generator.h>
#include <frame.h>
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
 * \param buf_len    The length of a mixing buffer -- must be > \c 0.
 * \param mix_rate   The mixing rate -- must be > \c 0.
 *
 * \return   The new Instrument if successful, or \c NULL if memory allocation
 *           failed.
 */
Instrument* new_Instrument(uint32_t buf_len, uint32_t mix_rate);


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
 * Parses an Instrument-level value from a textual description.
 *
 * \param ins      The Instrument -- must not be \c NULL.
 * \param subkey   The subkey -- must not be \c NULL.
 * \param str      The textual description.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will not be
 *           modified if memory allocation failed.
 */
bool Instrument_parse_value(Instrument* ins,
                            const char* subkey,
                            char* str,
                            Read_state* state);


/**
 * Gets the Instrument parameters of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Instrument parameters.
 */
Instrument_params* Instrument_get_params(Instrument* ins);


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
 * Gets the Generator table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Generator table.
 */
Gen_table* Instrument_get_gens(Instrument* ins);


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
//void Instrument_del_gen(Instrument* ins, int index);


/**
 * Gets an Effect of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Effect -- must be >= \c 0 and
 *                < \c KQT_INST_EFFECTS_MAX.
 *
 * \return   The Effect if one exists, otherwise \c NULL.
 */
Effect* Instrument_get_effect(Instrument* ins, int index);


/**
 * Gets the Effect table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Effect table.
 */
Effect_table* Instrument_get_effects(Instrument* ins);


/**
 * Sets the Connections of the Instrument.
 *
 * Previously set Connections will be removed if found.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param graph   The Connections, or \c NULL.
 */
void Instrument_set_connections(Instrument* ins, Connections* graph);


/**
 * Gets the Connections of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Connections, or \c NULL if none exist.
 */
Connections* Instrument_get_connections(Instrument* ins);


/**
 * Destroys an existing Instrument.
 *
 * \param ins   The Instrument, or \c NULL.
 */
void del_Instrument(Instrument* ins);


#endif // K_INSTRUMENT_H


