

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <Decl.h>
#include <devices/Effect.h>
#include <devices/Gen_table.h>
#include <devices/Generator.h>
#include <devices/Instrument_params.h>
#include <devices/param_types/Envelope.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <module/Effect_table.h>
#include <module/Scale.h>
#include <player/Voice_state.h>
#include <string/Streader.h>


typedef struct Instrument Instrument;


#define INS_DEFAULT_FORCE (0)
#define INS_DEFAULT_FORCE_VAR (0)
#define INS_DEFAULT_SCALE_INDEX (-1)


/**
 * Create a new Instrument.
 *
 * \return   The new Instrument if successful, or \c NULL if memory allocation
 *           failed.
 */
Instrument* new_Instrument(void);


/**
 * Parse an Instrument header from a textual description.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 * \param sr    The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_parse_header(Instrument* ins, Streader* sr);


/**
 * Parse an Instrument-level value from a textual description.
 *
 * \param ins      The Instrument -- must not be \c NULL.
 * \param subkey   The subkey -- must not be \c NULL.
 * \param sr       The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_parse_value(Instrument* ins, const char* subkey, Streader* sr);


/**
 * Get the Instrument parameters of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Instrument parameters.
 */
Instrument_params* Instrument_get_params(Instrument* ins);


/**
 * Get a Generator of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Generator -- must be >= \c 0 and
 *                < \c KQT_GENERATORS_MAX.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
const Generator* Instrument_get_gen(const Instrument* ins, int index);


/**
 * Get the Generator table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Generator table.
 */
Gen_table* Instrument_get_gens(Instrument* ins);


/**
 * Remove a Generator of the Instrument.
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
 * Get an Effect of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Effect -- must be >= \c 0 and
 *                < \c KQT_INST_EFFECTS_MAX.
 *
 * \return   The Effect if one exists, otherwise \c NULL.
 */
const Effect* Instrument_get_effect(const Instrument* ins, int index);


/**
 * Get the Effect table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Effect table.
 */
Effect_table* Instrument_get_effects(Instrument* ins);


/**
 * Set the Connections of the Instrument.
 *
 * Previously set Connections will be removed if found.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param graph   The Connections, or \c NULL.
 */
void Instrument_set_connections(Instrument* ins, Connections* graph);


/**
 * Get the Connections of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Connections, or \c NULL if none exist.
 */
Connections* Instrument_get_connections(Instrument* ins);


/**
 * Destroy an existing Instrument.
 *
 * \param ins   The Instrument, or \c NULL.
 */
void del_Instrument(Instrument* ins);


#endif // K_INSTRUMENT_H


