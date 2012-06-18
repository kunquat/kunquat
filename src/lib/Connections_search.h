

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
 * Where the connection is located.
 *
 * Instrument and effect levels can be combined by bitwise OR.
 */
typedef enum
{
    CONNECTION_LEVEL_GLOBAL = 0,
    CONNECTION_LEVEL_INSTRUMENT = 1,
    CONNECTION_LEVEL_EFFECT = 2,
} Connection_level;


/**
 * Creates new Connections from a string.
 *
 * \param str         The textual description.
 * \param ins_level   Whether this is an instrument-level graph or not.
 * \param insts       The Instrument table -- must not be \c NULL.
 * \param effects     The Effect table -- must not be \c NULL.
 * \param dsps        The DSP table -- must not be \c NULL.
 * \param master      The global or Instrument master node
 *                    -- must not be \c NULL.
 * \param state       The Read state -- must not be \c NULL.
 *
 * \return   The new Connections if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Connections* new_Connections_from_string(char* str,
                                         Connection_level level,
                                         Ins_table* insts,
                                         Effect_table* effects,
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


#endif // K_CONNECTIONS_SEARCH_H


