

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


#ifndef KQT_DEVICE_NODE_H
#define KQT_DEVICE_NODE_H


#include <init/Au_table.h>
#include <init/devices/Device.h>
#include <player/Device_states.h>
#include <player/Voice_group.h>

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
 * Initialise all Audio buffers in the Device node and its subgraph.
 *
 * \param node      The Device node -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_init_buffers_simple(const Device_node* node, Device_states* dstates);


/**
 * Initialise the graphs of the Effects in the subgraph.
 *
 * \param node      The Device node -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_node_init_effect_buffers(const Device_node* node, Device_states* dstates);


/**
 * Reset subgraph starting from the Device node.
 *
 * TODO: Remove this work-around after Audio units have their own Device states!
 *
 * \param node      The Device node -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
void Device_node_reset_subgraph(const Device_node* node, Device_states* dstates);


/**
 * Process a Voice group in the Device node and its subgraph.
 *
 * \param node         The Device node -- must not be \c NULL.
 * \param vgroup       The Voice group -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed.
 * \param buf_stop     The stop index of the buffer area to be processed.
 * \param audio_rate   The audio rate -- must be > \c 0.
 * \param tempo        The current tempo -- must be > \c 0.
 *
 * \return   The stop index of complete frames rendered to voice buffers. This
 *           is always within range [\a buf_start, \a buf_stop]. If the stop
 *           index is < \a buf_stop, the note has ended.
 */
int32_t Device_node_process_voice_group(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo);


/**
 * Add Voice signals to mixed signal input buffers.
 *
 * \param node        The Device node -- must not be \c NULL.
 * \param vgroup      The Voice group -- must not be \c NULL.
 * \param dstates     The Device states -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Device_node_mix_voice_signals(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process mixed signals in the Device node and its subgraph.
 *
 * \param node         The Device node -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be mixed -- must
 *                     be less than the buffer size.
 * \param buf_stop     The stop index of the buffer area to be mixed -- must
 *                     be less than or equal to the buffer size.
 * \param audio_rate   The mixing frequency -- must be > \c 0.
 * \param tempo        The tempo -- must be finite and > \c 0.
 */
void Device_node_process_mixed_signals(
        const Device_node* node,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo);


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


