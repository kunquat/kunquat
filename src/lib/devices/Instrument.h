

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <devices/Instrument_params.h>
#include <devices/param_types/Envelope.h>
#include <devices/Proc_table.h>
#include <devices/Processor.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <module/Scale.h>
#include <player/Voice_state.h>
#include <string/Streader.h>


#define INS_DEFAULT_GLOBAL_FORCE 0
#define INS_DEFAULT_FORCE 0
#define INS_DEFAULT_FORCE_VAR 0
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
 * Get a Processor of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Processor -- must be >= \c 0 and
 *                < \c KQT_PROCESSORS_MAX.
 *
 * \return   The Processor if found, otherwise \c NULL.
 */
const Processor* Instrument_get_proc(const Instrument* ins, int index);


/**
 * Get the Processor table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Processor table.
 */
Proc_table* Instrument_get_procs(Instrument* ins);


/**
 * Get an Instrument contained within the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the Instrument -- must be >= \c 0 and
 *                < \c KQT_INSTRUMENTS_MAX.
 *
 * \return   The Instrument if one exists, otherwise \c NULL.
 */
const Instrument* Instrument_get_ins(const Instrument* ins, int index);


/**
 * Get the Instrument table of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Instrument table.
 */
Ins_table* Instrument_get_insts(Instrument* ins);


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
const Connections* Instrument_get_connections(const Instrument* ins);


/**
 * Get the mutable Connections of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Connections, or \c NULL if none exist.
 */
Connections* Instrument_get_connections_mut(const Instrument* ins);


/**
 * Prepare the Connections of the Instrument.
 *
 * This function assumes that the outer input and output buffers of the Instrument
 * have been allocated.
 *
 * \param ins      The Instrument -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Instrument_prepare_connections(const Instrument* ins, Device_states* states);


/**
 * Get the input interface of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The input interface.
 */
const Device* Instrument_get_input_interface(const Instrument* ins);


/**
 * Get the output interface of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The output interface.
 */
const Device* Instrument_get_output_interface(const Instrument* ins);


/**
 * Destroy an existing Instrument.
 *
 * \param ins   The Instrument, or \c NULL.
 */
void del_Instrument(Instrument* ins);


#endif // K_INSTRUMENT_H


