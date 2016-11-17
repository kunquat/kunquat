

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CONNECTIONS_H
#define KQT_CONNECTIONS_H


#include <decl.h>
#include <init/Au_table.h>
#include <init/Device_node.h>
#include <init/devices/Device.h>
#include <player/Device_states.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


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
    CONNECTION_LEVEL_AU = 1,
} Connection_level;


/**
 * Create new Connections from a string.
 *
 * \param sr         The Streader of the JSON input -- must not be \c NULL.
 * \param level      The connection level.
 * \param au_table   The Audio unit table -- must not be \c NULL.
 * \param master     The global or Audio unit master node
 *                   -- must not be \c NULL.
 *
 * \return   The new Connections if successful, otherwise \c NULL.
 */
Connections* new_Connections_from_string(
        Streader* sr, Connection_level level, Au_table* au_table, Device* master);


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
const Device_node* Connections_get_master(const Connections* graph);


/**
 * Print the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param out     The output file -- must not be \c NULL.
 */
void Connections_print(const Connections* graph, FILE* out);


/**
 * Destroy existing Connections.
 *
 * \param graph   The Connections, or \c NULL.
 */
void del_Connections(Connections* graph);


#endif // KQT_CONNECTIONS_H


