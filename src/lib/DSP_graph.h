

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


#ifndef K_DSP_GRAPH_H
#define K_DSP_GRAPH_H


#include <DSP_vertex.h>
#include <File_base.h>


/**
 * This structure contains connection information of the DSP nodes.
 */
typedef struct DSP_graph DSP_graph;


/**
 * Creates a new DSP graph from a string.
 *
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new DSP graph if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
DSP_graph* new_DSP_graph_from_string(char* str, Read_state* state);


/**
 * Destroys an existing DSP graph.
 *
 * \param graph   The DSP graph -- must not be \c NULL.
 */
void del_DSP_graph(DSP_graph* graph);


#endif // K_DSP_GRAPH_H


