

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


#ifndef K_CONNECTIONS_SEARCH_H
#define K_CONNECTIONS_SEARCH_H


#include <Connections.h>
#include <Device.h>
#include <Device_node.h>
#include <DSP_table.h>
#include <Ins_table.h>


/**
 * These are auxiliary algorithms used with Connections.
 */


/**
 * Creates new Connections from a string.
 *
 * \param str         The textual description.
 * \param ins_level   Whether this is an instrument-level graph or not.
 * \param insts       The Instrument table -- must not be \c NULL.
 * \param dsps        The DSP table -- must not be \c NULL.
 * \param master      The global or Instrument master node
 *                    -- must not be \c NULL.
 * \param state       The Read state -- must not be \c NULL.
 *
 * \return   The new Connections if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Connections* new_Connections_from_string(char* str,
                                         bool ins_level,
                                         Ins_table* insts,
                                         DSP_table* dsps,
                                         Device* master,
                                         Read_state* state);


/**
 * Retrieves the master Device node of the Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   The master node if one exists, otherwise \c NULL.
 */
Device_node* Connections_get_master(Connections* graph);


/**
 * Prepares the Connections for mixing.
 *
 * \param graph    The Connections -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Connections_prepare(Connections* graph);


/**
 * Sets the appropriate Devices for the Connections.
 *
 * \param graph    The Connections -- must not be \c NULL.
 * \param master   The master Device -- must not be \c NULL.
 * \param insts    The Instrument table -- must not be \c NULL.
 * \param dsps     The DSP table -- must not be \c NULL.
 */
#if 0
void Connections_set_devices(Connections* graph,
                             Device* master,
                             Ins_table* insts,
                             DSP_table* dsps);
#endif


#endif // K_CONNECTIONS_SEARCH_H


