

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

#include <DSP_vertex.h>
#include <File_base.h>


/**
 * This structure contains connection information of the DSP nodes.
 */
typedef struct Connections Connections;


/**
 * Creates new Connections from a string.
 *
 * \param str         The textual description.
 * \param ins_level   Whether this is an instrument-level graph or not.
 * \param state       The Read state -- must not be \c NULL.
 *
 * \return   The new Connections if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Connections* new_Connections_from_string(char* str,
                                         bool ins_level,
                                         Read_state* state);


/**
 * Destroys existing Connections.
 *
 * \param graph   The Connections -- must not be \c NULL.
 */
void del_Connections(Connections* graph);


#endif // K_CONNECTIONS_H


