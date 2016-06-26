

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
#include <player/Voice_group.h>
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
 * \param au_level   Whether this is an audio unit level graph or not.
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
Device_node* Connections_get_master(Connections* graph);


/**
 * Prepare the Connections for mixing.
 *
 * \param graph     The Connections -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_prepare(const Connections* graph, Device_states* dstates);


/**
 * Initialise all Audio buffers in the Connections.
 *
 * \param graph     The Connections -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_init_buffers(const Connections* graph, Device_states* dstates);


/**
 * Clear the Audio buffers in the Connections.
 *
 * \param graph       The Connections -- must not be \c NULL.
 * \param dstates     The Device states -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be cleared -- must
 *                    be less than the buffer size.
 * \param buf_stop    The stop index of the buffer area to be cleared -- must
 *                    be less than or equal to the buffer size.
 */
void Connections_clear_buffers(
        const Connections* graph,
        Device_states* states,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process a Voice group inside the Connections.
 *
 * \param graph        The Connections -- must not be \c NULL.
 * \param vgroup       The Voice group -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed
 *                     -- must not be negative.
 * \param buf_stop     The stop index of the buffer area to be processed
 *                     -- must not be negative.
 * \param audio_rate   The audio rate -- must be > \c 0.
 * \param tempo        The current tempo -- must be finite and > \c 0.
 *
 * \return   The stop index of complete frames rendered to voice buffers. This
 *           is always within range [\a buf_start, \a buf_stop]. If the stop
 *           index is < \a buf_stop, the note has ended.
 */
int32_t Connections_process_voice_group(
        const Connections* graph,
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
 * \param graph       The Connections -- must not be \c NULL.
 * \param vgroup      The Voice group -- must not be \c NULL.
 * \param dstates     The Device states -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Connections_mix_voice_signals(
        const Connections* graph,
        Voice_group* vgroup,
        Device_states* dstates,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process mixed signals in the Connections.
 *
 * \param graph        The Connections -- must not be \c NULL.
 * \param hack_reset   Whether the Device states should be reset or not. TODO: fix this
 * \param dstates      The Device states -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed
 *                     -- must be less than the buffer size.
 * \param buf_stop     The stop index of the buffer area to be processed
 *                     -- must be less than or equal to the buffer size.
 * \param audio_rate   The audio rate -- must be > \c 0.
 * \param tempo        The tempo -- must be finite and > \c 0.
 */
void Connections_process_mixed_signals(
        const Connections* graph,
        bool hack_reset,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo);


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


