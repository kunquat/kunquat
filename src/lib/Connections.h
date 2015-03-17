

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


#ifndef K_CONNECTIONS_H
#define K_CONNECTIONS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Decl.h>
#include <Device_node.h>
#include <devices/Device.h>
#include <module/Ins_table.h>
#include <player/Device_states.h>
#include <string/Streader.h>


/**
 * This structure contains connection information of the Devices.
 */
struct Connections;


/**
 * Where the connection is located.
 */
typedef enum
{
    CONNECTION_LEVEL_GLOBAL = 0,
    CONNECTION_LEVEL_INSTRUMENT = 1,
} Connection_level;


/**
 * Create new Connections from a string.
 *
 * \param sr          The Streader of the JSON input -- must not be \c NULL.
 * \param ins_level   Whether this is an instrument-level graph or not.
 * \param insts       The Instrument table -- must not be \c NULL.
 * \param master      The global or Instrument master node
 *                    -- must not be \c NULL.
 *
 * \return   The new Connections if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Connections* new_Connections_from_string(
        Streader* sr,
        Connection_level level,
        Ins_table* insts,
        Device* master);


/**
 * Check that each connection is between existing ports.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param err     Error string destination -- must not be \c NULL.
 *
 * \return   \c true if all connected ports exist, otherwise \c false.
 */
bool Connections_check_connections(
        const Connections* graph, char err[DEVICE_CONNECTION_ERROR_LENGTH_MAX]);


/**
 * Retrieve the master Device node of the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   The master node if one exists, otherwise \c NULL.
 */
Device_node* Connections_get_master(Connections* graph);


/**
 * Prepare the Connections for mixing.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_prepare(Connections* graph, Device_states* states);


/**
 * Initialise all Audio buffers in the Connections.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_init_buffers(Connections* graph, Device_states* states);


/**
 * Clear the Audio buffers in the Connections.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param start    The first frame to be cleared -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be cleared -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be mixed.
 */
void Connections_clear_buffers(
        Connections* graph,
        Device_states* states,
        uint32_t start,
        uint32_t until);


/**
 * Mix the audio in the Connections.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param start    The first frame to be mixed -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be mixed -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be mixed.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Connections_mix(
        Connections* graph,
        Device_states* device_states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


/**
 * Print the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param out     The output file -- must not be \c NULL.
 */
void Connections_print(Connections* graph, FILE* out);


/**
 * Destroy existing Connections.
 *
 * \param graph   The Connections, or \c NULL.
 */
void del_Connections(Connections* graph);


#endif // K_CONNECTIONS_H


