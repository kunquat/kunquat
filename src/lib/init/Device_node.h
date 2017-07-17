

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_NODE_H
#define KQT_DEVICE_NODE_H


#include <decl.h>
#include <init/devices/Device.h>
#include <init/devices/port_type.h>
#include <player/Device_states.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#define KQT_DEVICE_NODE_NAME_MAX 32
#define DEVICE_CONNECTION_ERROR_LENGTH_MAX 256


/**
 * A node in Connections. This contains a reference to the Device, not the
 * Device itself.
 *
 * The structure in memory allows Device nodes to be compared against strings.
 */
typedef struct Device_node Device_node;


typedef enum
{
    DEVICE_NODE_TYPE_MASTER    = 1,
    DEVICE_NODE_TYPE_PROCESSOR = 2,
    DEVICE_NODE_TYPE_AU        = 8,
} Device_node_type;


typedef struct Connection
{
    Device_node* node;       ///< The neighbour node.
    int port;                ///< The port of the neighbour node.
    struct Connection* next;
} Connection;


/**
 * Create a new Device node.
 *
 * \param name       The name of the node -- must not be \c NULL.
 * \param au_table   The Audio unit table -- must not be \c NULL.
 * \param master     The global or Audio unit master Device
 *                   -- must not be \c NULL.
 *
 * \return   The new Device node if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_node* new_Device_node(const char* name, Au_table* au_table, const Device* master);


/**
 * Compare two existing Device nodes.
 *
 * \param n1   The first Device node -- must not be \c NULL.
 * \param n2   The second Device node -- must not be \c NULL.
 *
 * \return   An integer less than, equal to, or greater than zero if \a v1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a v2.
 */
int Device_node_cmp(const Device_node* n1, const Device_node* n2);


/**
 * Check that each connection to the Device node is between existing ports.
 *
 * \param node   The Device node -- must not be \c NULL.
 * \param err    Error string destination -- must not be \c NULL.
 *
 * \return   \c true if all connected ports exist, otherwise \c false.
 */
bool Device_node_check_connections(
        const Device_node* node, char err[DEVICE_CONNECTION_ERROR_LENGTH_MAX]);


/**
 * Get the name of the corresponding Device.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The name.
 */
const char* Device_node_get_name(const Device_node* node);


/**
 * Get the Device of the Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The Device.
 */
const Device* Device_node_get_device(const Device_node* node);


/**
 * Get the last receive port number with a defined connection.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The last receive port number, or \c -1 if \a node has no receive
 *           connections.
 */
int Device_node_get_last_receive_port(const Device_node* node);


/**
 * Get received connections to a port of a Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * return   The first connection if one exists, otherwise \c NULL.
 */
const Connection* Device_node_get_received(const Device_node* node, int port);


/**
 * Get the type of the Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The type of the Device node.
 */
Device_node_type Device_node_get_type(const Device_node* node);


/**
 * Get the Audio unit contained within the Device node.
 *
 * \param node   The Device node -- must not be \c NULL and must have type
 *               \c DEVICE_NODE_TYPE_AU.
 *
 * \return   The Audio unit if one exists, otherwise \c NULL.
 */
Audio_unit* Device_node_get_au_mut(const Device_node* node);


/**
 * Connect two Device nodes.
 *
 * \param receiver    The Device node that receives audio -- must not
 *                    be \c NULL.
 * \param rec_port    The receive port number of \a receiver -- must be
 *                    >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 * \param sender      The Device node that sends audio -- must not be \c NULL.
 * \param send_port   The send port number of \a sender -- must be >= \c 0
 *                    and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_connect(
        Device_node* receiver, int rec_port, Device_node* sender, int send_port);


/**
 * Disconnect a Device from the Device node.
 *
 * \param node     The Device node -- must not be \c NULL.
 * \param device   The Device to be disconnected -- must not be \c NULL.
 */
void Device_node_disconnect(Device_node* node, const Device* device);


/**
 * Replace a Device in the connections of the Device node.
 *
 * \param node         The Device node -- must not be \c NULL.
 * \param old_device   The old Device -- must not be \c NULL.
 * \param new_device   The new Device -- must not be \c NULL, equal to
 *                     \a old_device or in the connections of \a node.
 */
void Device_node_replace(
        Device_node* node, const Device* old_device, const Device* new_device);


/**
 * Reset cycle test status of the Device node.
 *
 * This function does not recurse to connected nodes.
 *
 * \param node   The Device node -- must not be \c NULL.
 */
void Device_node_reset_cycle_test_state(Device_node* node);


/**
 * Search for a cycle in the path starting from this Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   \c true if a cycle was found, otherwise \c false.
 */
bool Device_node_cycle_in_path(Device_node* node);


/**
 * Get the maximum number of Device nodes connected in chain from the Device node.
 *
 * The depth includes any subgraphs of contained Audio units.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The maximum number of Device nodes in a single connection chain.
 */
int Device_node_get_subgraph_depth(const Device_node* node);


/**
 * Print a textual description of the Device node and its neighbours.
 *
 * \param node   The Device node -- must not be \c NULL.
 * \param out    The output file -- must not be \c NULL.
 */
void Device_node_print(const Device_node* node, FILE* out);


/**
 * Destroy an existing Device node.
 *
 * \param node   The Device node, or \c NULL.
 */
void del_Device_node(Device_node* node);


#endif // KQT_DEVICE_NODE_H


