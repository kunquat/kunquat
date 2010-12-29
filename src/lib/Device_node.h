

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DEVICE_NODE_H
#define K_DEVICE_NODE_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Device.h>
#include <Ins_table.h>
#include <math_common.h>


/**
 * The state of a Device node during a search. These are sometimes referred to
 * as the colours white, gray and black.
 */
typedef enum
{
    DEVICE_NODE_STATE_NEW = 0,
    DEVICE_NODE_STATE_REACHED,
    DEVICE_NODE_STATE_VISITED
} Device_node_state;


#define KQT_DEVICE_NODE_NAME_MAX (32)


/**
 * A node in Connections. This contains a reference to the Device, not the
 * Device itself.
 *
 * The structure in memory allows Device nodes to be compared against strings.
 */
typedef struct Device_node Device_node;


/**
 * Creates a new Device node.
 *
 * \param name   The name of the node -- must not be \c NULL.
 *
 * \return   The new Device node if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_node* new_Device_node(const char* name);


/**
 * Compares two existing Device nodes.
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
 * Resets the Device node and its subgraph.
 *
 * This function assumes that if the underlying Connections graph is not
 * reset, all its nodes have been marked at least reached.
 *
 * \param node   The Device node -- must not be \c NULL.
 */
void Device_node_reset(Device_node* node);


/**
 * Sets the devices starting from the given Device node.
 *
 * \param node     The Device node -- must not be \c NULL.
 * \param master   The master Device -- must not be \c NULL.
 * \param insts    The Instrument table -- must not be \c NULL.
 * \param dsps     The DSP table -- must not be \c NULL.
 */
void Device_node_set_devices(Device_node* node,
                             Device* master,
                             Ins_table* insts,
                             DSP_table* dsps);


/**
 * Initialises all Audio buffers in the Device node and its subgraph.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_init_buffers_simple(Device_node* node);


/**
 * Removes all direct Audio buffers from the Device node and its subgraph.
 *
 * \param node   The Device node -- must not be \c NULL.
 */
void Device_node_remove_direct_buffers(Device_node* node);


/**
 * Initialises all the necessary input Audio buffers in the Device node and
 * its subgraph.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_init_input_buffers(Device_node* node);


/**
 * Initialises all the remaining Audio buffers in the Device node and its
 * subgraph.
 *
 * \param node         The Device node -- must not be \c NULL.
 * \param send_port    The send port of the connection that the caller
 *                     used to reach \a node -- must be >= \c 0 and
 *                     <= \c KQT_DEVICE_PORTS_MAX.
 * \param suggestion   A suggestion for the Device node to use as its
 *                     send buffer for \a send_port, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_init_buffers_by_suggestion(Device_node* node,
                                            int send_port,
                                            Audio_buffer* suggestion);


/**
 * Clears the audio buffers in the Device node and its subgraph.
 *
 * \param node    The Device node -- must not be \c NULL.
 * \param start   The first frame to be cleared -- must be less than the
 *                buffer size.
 * \param until   The first frame not to be cleared -- must be less than or
 *                equal to the buffer size. If \a until <= \a start, nothing
 *                will be cleared.
 */
void Device_node_clear_buffers(Device_node* node,
                               uint32_t start,
                               uint32_t until);


/**
 * Mixes audio in the Device node and its subgraph.
 *
 * \param node    The Device node -- must not be \c NULL.
 * \param start   The first frame to be mixed -- must be less than the
 *                buffer size.
 * \param until   The first frame not to be mixed -- must be less than or
 *                equal to the buffer size. If \a until <= \a start, nothing
 *                will be mixed.
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param tempo   The tempo -- must be > \c 0 and finite.
 */
void Device_node_mix(Device_node* node,
                     uint32_t start,
                     uint32_t until,
                     uint32_t freq,
                     double tempo);


/**
 * Gets the name of the corresponding Device.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The name.
 */
char* Device_node_get_name(Device_node* node);


/**
 * Gets the Device of the Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The Device.
 */
Device* Device_node_get_device(Device_node* node);


/**
 * Sets the node state of the Device node.
 *
 * \param node    The Device node -- must not be \c NULL.
 * \param state   The state -- must be valid.
 */
void Device_node_set_state(Device_node* node, Device_node_state state);


/**
 * Gets the node state of the Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   The state.
 */
Device_node_state Device_node_get_state(Device_node* node);


/**
 * Connects two Device nodes.
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
bool Device_node_connect(Device_node* receiver,
                         int rec_port,
                         Device_node* sender,
                         int send_port);


/**
 * Disconnects a Device from the Device node.
 *
 * \param node     The Device node -- must not be \c NULL.
 * \param device   The Device to be disconnected -- must not be \c NULL.
 */
void Device_node_disconnect(Device_node* node, Device* device);


/**
 * Replaces a Device in the connections of the Device node.
 *
 * \param node         The Device node -- must not be \c NULL.
 * \param old_device   The old Device -- must not be \c NULL.
 * \param new_device   The new Device -- must not be \c NULL, equal to
 *                     \a old_device or in the connections of \a node.
 */
void Device_node_replace(Device_node* node,
                         Device* old_device,
                         Device* new_device);


/**
 * Gets the first in a list of Device nodes that send audio to this Device node.
 *
 * \param node        The Device node -- must not be \c NULL.
 * \param rec_port    The receive port number -- must be >= \c 0 and
 *                    < \c KQT_DEVICE_PORTS_MAX.
 * \param send_port   A pointer where the send port of the returned node
 *                    will be stored, or \c NULL.
 *
 * \return   The first sender if one exists, otherwise \c NULL.
 */
Device_node* Device_node_get_sender(Device_node* node,
                                    int rec_port,
                                    int* send_port);


/**
 * Gets the first in a list of Device nodes that receive audio from this Device node.
 *
 * \param node        The Device node -- must not be \c NULL.
 * \param send_port   The send port number -- must be >= \c 0 and
 *                    < \c KQT_DEVICE_PORTS_MAX.
 * \param rec_port    A pointer where the receive port of the returned
 *                    node will be stored, or \c NULL.
 *
 * \return   The first receiver if one exists, otherwise \c NULL.
 */
Device_node* Device_node_get_receiver(Device_node* node,
                                      int send_port,
                                      int* rec_port);


/**
 * Gets the next neighbour from the last requested list of a Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 * \param port   A pointer where the port of the returned node
 *               will be stored, or \c NULL.
 *
 * \return   The next neighbour if one exists, otherwise \c NULL.
 */
Device_node* Device_node_get_next(Device_node* node, int* port);


/**
 * Searches for a cycle in the path starting from this Device node.
 *
 * \param node   The Device node -- must not be \c NULL.
 *
 * \return   \c true if a cycle was found, otherwise \c false.
 */
bool Device_node_cycle_in_path(Device_node* node);


/**
 * Prints a textual description of the Device node and its neighbours.
 *
 * \param node   The Device node -- must not be \c NULL.
 * \param out    The output file -- must not be \c NULL.
 */
void Device_node_print(Device_node* node, FILE* out);


/**
 * Destroys an existing Device node.
 *
 * \param node   The Device node, or \c NULL.
 */
void del_Device_node(Device_node* node);


#endif // K_DEVICE_NODE_H


