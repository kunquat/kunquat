

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


#ifndef K_DSP_VERTEX_H
#define K_DSP_VERTEX_H


#include <stdbool.h>

#include <math_common.h>


/**
 * The state of a DSP vertex during a search. These are sometimes referred to
 * as the colours white, gray and black.
 */
typedef enum
{
    DSP_VERTEX_STATE_NEW = 0,
    DSP_VERTEX_STATE_REACHED,
    DSP_VERTEX_STATE_VISITED
} DSP_vertex_state;


/**
 * A vertex in a DSP graph. This contains an index of the DSP table, not the
 * DSP node itself.
 *
 * The structure in memory allows DSP vertices to be compared against values
 * of type int.
 */
typedef struct DSP_vertex DSP_vertex;


#define DSP_VERTEX_MASTER (-1)
#define DSP_VERTEX_OTHERS (-2)
#define DSP_VERTEX_MIN    MIN(DSP_VERTEX_MASTER, DSP_VERTEX_OTHERS)


/**
 * Creates a new DSP vertex.
 *
 * \param index   The index of the corresponding DSP node in the DSP table
 *                -- must be >= \c 0 and < \c KQT_DSP_NODES_MAX, or either
 *                \c DSP_VERTEX_MASTER or \c DSP_VERTEX_OTHERS.
 *
 * \return   The new DSP vertex if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP_vertex* new_DSP_vertex(int index);


/**
 * Compares two existing DSP vertices.
 *
 * \param v1   The first DSP vertex -- must not be \c NULL.
 * \param v2   The second DSP vertex -- must not be \c NULL.
 *
 * \return   An integer less than, equal to, or greater than zero if \a v1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a v2.
 */
int DSP_vertex_cmp(const DSP_vertex* v1, const DSP_vertex* v2);


/**
 * Gets the index of the corresponding DSP node in the DSP table.
 *
 * \param vertex   The DSP vertex -- must not be \c NULL.
 *
 * \return   The index. \c -1 indicates that this vertex represents the
 *           master node, and \c -2 represents the "others" node.
 */
int DSP_vertex_get_index(DSP_vertex* vertex);


/**
 * Sets the vertex state of the DSP vertex.
 *
 * \param vertex   The DSP vertex -- must not be \c NULL.
 * \param state    The state -- must be valid.
 */
void DSP_vertex_set_state(DSP_vertex* vertex, DSP_vertex_state state);


/**
 * Gets the vertex state of the DSP vertex.
 *
 * \param vertex   The DSP vertex -- must not be \c NULL.
 *
 * \return   The state.
 */
DSP_vertex_state DSP_vertex_get_state(DSP_vertex* vertex);


/**
 * Adds a neighbour of a DSP vertex.
 *
 * \param vertex     The DSP vertex -- must not be \c NULL.
 * \param in_port    The input port number of \a vertex -- must be >= \c 0 and
 *                   < \c KQT_DSP_PORTS_MAX.
 * \param adj        The neighbouring DSP vertex -- must not be \c NULL.
 * \param out_port   The output port number of \a adj -- must be >= \c 0 and
 *                   < \c KQT_DSP_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool DSP_vertex_set_adjacent(DSP_vertex* vertex,
                             int in_port,
                             DSP_vertex* adj,
                             int out_port);


/**
 * Gets the first in a list of neighbours of a DSP vertex.
 *
 * \param vertex     The DSP vertex -- must not be \c NULL.
 * \param in_port    The input port number -- must be >= \c 0 and
 *                   < \c KQT_DSP_PORTS_MAX.
 * \param out_port   A pointer where the output port of the returned vertex
 *                   will be stored, or \c NULL.
 *
 * \return   The first neighbour if one exists, otherwise \c NULL.
 */
DSP_vertex* DSP_vertex_get_adjacent(DSP_vertex* vertex,
                                    int in_port,
                                    int* out_port);


/**
 * Gets the next neighbour from the last requested list of a DSP vertex.
 *
 * \param vertex     The DSP vertex -- must not be \c NULL.
 * \param out_port   A pointer where the output port of the returned vertex
 *                   will be stored, or \c NULL.
 *
 * \return   The next neighbour if one exists, otherwise \c NULL.
 */
DSP_vertex* DSP_vertex_get_next(DSP_vertex* vertex, int* out_port);


/**
 * Searches for a cycle in the path starting from this DSP vertex.
 *
 * \param vertex   The DSP vertex -- must not be \c NULL.
 *
 * \return   \c true if a cycle was found, otherwise \c false.
 */
bool DSP_vertex_cycle_in_path(DSP_vertex* vertex);


/**
 * Destroys an existing DSP vertex.
 *
 * \param vertex   The DSP vertex -- must not be \c NULL.
 */
void del_DSP_vertex(DSP_vertex* vertex);


#endif // K_DSP_VERTEX_H


