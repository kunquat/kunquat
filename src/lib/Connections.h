

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


#ifndef K_CONNECTIONS_H
#define K_CONNECTIONS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <devices/Device.h>
#include <player/Device_states.h>


/**
 * This structure contains connection information of the Devices.
 */
typedef struct Connections Connections;


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


