

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


#ifndef K_AUDIO_UNIT_H
#define K_AUDIO_UNIT_H


#include <Decl.h>
#include <devices/Au_control_vars.h>
#include <devices/Au_params.h>
#include <devices/param_types/Envelope.h>
#include <devices/Proc_table.h>
#include <devices/Processor.h>
#include <kunquat/limits.h>
#include <module/Scale.h>
#include <string/Streader.h>

#include <stdbool.h>


#define AU_DEFAULT_GLOBAL_FORCE 0
#define AU_DEFAULT_FORCE 0
#define AU_DEFAULT_FORCE_VAR 0
#define AU_DEFAULT_GLOBAL_LOWPASS 100
#define AU_DEFAULT_SCALE_INDEX (-1)


typedef enum
{
    AU_TYPE_INVALID = 0,
    AU_TYPE_INSTRUMENT,
    AU_TYPE_EFFECT,
} Au_type;


/**
 * Create a new Audio unit.
 *
 * \return   The new Audio unit if successful, or \c NULL if memory allocation
 *           failed.
 */
Audio_unit* new_Audio_unit(void);


/**
 * Set the type of the Audio unit.
 *
 * \param au     The Audio unit -- must not be \c NULL.
 * \param type   The Audio unit type -- must be valid.
 */
void Audio_unit_set_type(Audio_unit* au, Au_type type);


/**
 * Get the type of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The type of the Audio unit.
 */
Au_type Audio_unit_get_type(const Audio_unit* au);


/**
 * Parse an Audio unit header from a textual description.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_unit_parse_header(Audio_unit* au, Streader* sr);


/**
 * Get the Audio unit parameters of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The Audio unit parameters.
 */
Au_params* Audio_unit_get_params(Audio_unit* au);


/**
 * Get a Processor of the Audio unit.
 *
 * \param au      The Audio unit -- must not be \c NULL.
 * \param index   The index of the Processor -- must be >= \c 0 and
 *                < \c KQT_PROCESSORS_MAX.
 *
 * \return   The Processor if found, otherwise \c NULL.
 */
const Processor* Audio_unit_get_proc(const Audio_unit* au, int index);


/**
 * Get the Processor table of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The Processor table.
 */
Proc_table* Audio_unit_get_procs(const Audio_unit* au);


/**
 * Get an Audio unit contained within the Audio unit.
 *
 * \param au      The Audio unit -- must not be \c NULL.
 * \param index   The index of the Audio unit -- must be >= \c 0 and
 *                < \c KQT_AUDIO_UNITS_MAX.
 *
 * \return   The Audio unit if one exists, otherwise \c NULL.
 */
const Audio_unit* Audio_unit_get_au(const Audio_unit* au, int index);


/**
 * Get the Audio unit table of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The Audio unit table.
 */
Au_table* Audio_unit_get_au_table(Audio_unit* au);


/**
 * Set the Connections of the Audio unit.
 *
 * Previously set Connections will be removed if found.
 *
 * \param au      The Audio unit -- must not be \c NULL.
 * \param graph   The Connections, or \c NULL.
 */
void Audio_unit_set_connections(Audio_unit* au, Connections* graph);


/**
 * Get the Connections of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The Connections, or \c NULL if none exist.
 */
const Connections* Audio_unit_get_connections(const Audio_unit* au);


/**
 * Get the mutable Connections of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The Connections, or \c NULL if none exist.
 */
Connections* Audio_unit_get_connections_mut(const Audio_unit* au);


/**
 * Prepare the Connections of the Audio unit.
 *
 * This function assumes that the outer input and output buffers of the Audio unit
 * have been allocated.
 *
 * \param au       The Audio unit -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Audio_unit_prepare_connections(const Audio_unit* au, Device_states* states);


/**
 * Get the input interface of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The input interface.
 */
const Device* Audio_unit_get_input_interface(const Audio_unit* au);


/**
 * Get the output interface of the Audio unit.
 *
 * \param au   The Audio unit -- must not be \c NULL.
 *
 * \return   The output interface.
 */
const Device* Audio_unit_get_output_interface(const Audio_unit* au);


/**
 * Set the control variable mapping of the Audio unit.
 *
 * \param au                The Audio unit -- must not be \c NULL.
 * \param au_control_vars   The Audio unit control variables, or \c NULL.
 */
void Audio_unit_set_control_vars(Audio_unit* au, Au_control_vars* au_control_vars);


/**
 * Destroy an existing Audio unit.
 *
 * \param au   The Audio unit, or \c NULL.
 */
void del_Audio_unit(Audio_unit* au);


#endif // K_AUDIO_UNIT_H


