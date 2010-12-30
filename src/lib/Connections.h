

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


#ifndef K_CONNECTIONS_H
#define K_CONNECTIONS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Device.h>
#include <File_base.h>


/**
 * This structure contains connection information of the Devices.
 */
typedef struct Connections Connections;


/**
 * Initialises all Audio buffers in the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_init_buffers_simple(Connections* graph);


/**
 * Initialises Audio buffers in the Connections.
 *
 * This function uses a more sophisticated approach to allocating buffers than
 * Connections_init_buffers_simple. Typically, it uses far less buffers, which
 * requires less mixing and saves a bit of memory.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_init_buffers(Connections* graph);


/**
 * Clears the Audio buffers in the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param start   The first frame to be cleared -- must be less than the
 *                buffer size.
 * \param until   The first frame not to be cleared -- must be less than or
 *                equal to the buffer size. If \a until <= \a start, nothing
 *                will be mixed.
 */
void Connections_clear_buffers(Connections* graph,
                               uint32_t start,
                               uint32_t until);


/**
 * Mixes the audio in Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param start   The first frame to be mixed -- must be less than the
 *                buffer size.
 * \param until   The first frame not to be mixed -- must be less than or
 *                equal to the buffer size. If \a until <= \a start, nothing
 *                will be mixed.
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param tempo   The tempo -- must be > \c 0 and finite.
 */
void Connections_mix(Connections* graph,
                     uint32_t start,
                     uint32_t until,
                     uint32_t freq,
                     double tempo);


/**
 * Disconnects a Device from the Connections.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param device   The Device -- must not be \c NULL.
 */
//void Connections_disconnect(Connections* graph, Device* device);


/**
 * Replaces a Device with another in the Connections.
 *
 * \param graph        The Connections -- must not be \c NULL.
 * \param old_device   The old Device -- must not be \c NULL.
 * \param new_device   The new Device -- must not be \c NULL, equal to
 *                     \a old_device or already connected in \a graph.
 */
//void Connections_replace(Connections* graph,
//                         Device* old_device,
//                         Device* new_device);


/**
 * Prints the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 * \param out     The output file -- must not be \c NULL.
 */
void Connections_print(Connections* graph, FILE* out);


/**
 * Destroys existing Connections.
 *
 * \param graph   The Connections, or \c NULL.
 */
void del_Connections(Connections* graph);


#endif // K_CONNECTIONS_H


