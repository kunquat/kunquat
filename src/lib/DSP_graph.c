

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>

#include <AAtree.h>
#include <DSP_graph.h>
#include <DSP_vertex.h>

#include <xmemory.h>


struct DSP_graph
{
    AAtree* vertices;
    AAiter* iter;
};


/**
 * Resets the graph for searching purposes.
 *
 * \param graph   The DSP graph -- must not be \c NULL.
 */
static void DSP_graph_reset(DSP_graph* graph);


/**
 * Tells whether there is a cycle inside a DSP graph.
 *
 * All DSP graphs must be acyclic.
 *
 * \param graph   The DSP graph -- must not be \c NULL.
 *
 * \return   \c true if there is a cycle in \a graph, otherwise \c false.
 */
static bool DSP_graph_is_cyclic(DSP_graph* graph);


#define clean_if(expr, graph, vertex)   \
    if (true)                           \
    {                                   \
        if ((expr))                     \
        {                               \
            if (vertex != NULL)         \
            {                           \
                del_DSP_vertex(vertex); \
            }                           \
            del_DSP_graph(graph);       \
            return NULL;                \
        }                               \
    } else (void)0

DSP_graph* new_DSP_graph_from_string(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    DSP_graph* graph = xalloc(DSP_graph);
    if (graph == NULL)
    {
        return NULL;
    }
    graph->vertices = NULL;
    graph->iter = NULL;
    graph->vertices = new_AAtree((int (*)(const void*, const void*))DSP_vertex_cmp,
                                 (void (*)(void*))del_DSP_vertex);
    clean_if(graph->vertices == NULL, graph, NULL);
    graph->iter = new_AAiter(graph->vertices);
    clean_if(graph->iter == NULL, graph, NULL);

    DSP_vertex* master = new_DSP_vertex(-1);
    clean_if(master == NULL, graph, NULL);
    clean_if(!AAtree_ins(graph->vertices, master), graph, master);

    DSP_vertex* others = new_DSP_vertex(-2);
    clean_if(others == NULL, graph, NULL);
    clean_if(!AAtree_ins(graph->vertices, others), graph, others);

    if (str == NULL)
    {
        clean_if(!DSP_vertex_set_adjacent(master, 0, others, 0), graph, NULL);
        return graph;
    }
    
    str = read_const_char(str, '[', state);
    clean_if(state->error, graph, NULL);
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return graph;
    }
    Read_state_clear_error(state);

    bool expect_entry = true;
    while (expect_entry)
    {
        str = read_const_char(str, '[', state);
        int64_t src = 0;
        int64_t src_port = 0;
        str = read_const_char(str, '[', state);
        str = read_int(str, &src, state);
        str = read_const_char(str, ',', state);
        str = read_int(str, &src_port, state);
        str = read_const_char(str, ']', state);
        str = read_const_char(str, ',', state);
        int64_t dest = 0;
        int64_t dest_port = 0;
        str = read_const_char(str, '[', state);
        str = read_int(str, &dest, state);
        str = read_const_char(str, ',', state);
        str = read_int(str, &dest_port, state);
        str = read_const_char(str, ']', state);
        str = read_const_char(str, ']', state);

        clean_if(state->error, graph, NULL);
        
        if (src < DSP_VERTEX_MIN || src >= KQT_DSP_NODES_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP node index: %" PRId64, src);
        }
        if (dest < DSP_VERTEX_MIN || dest >= KQT_DSP_NODES_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP node index: %" PRId64, src);
        }
        if (src == DSP_VERTEX_MASTER)
        {
            Read_state_set_error(state, "DSP master node used as a source");
        }
        if (dest == DSP_VERTEX_OTHERS)
        {
            Read_state_set_error(state,
                    "DSP others node used as a destination");
        }
        if (src_port < 0 || src_port >= KQT_DSP_PORTS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP source port: %" PRId64, src_port);
        }
        if (dest_port < 0 || dest_port >= KQT_DSP_PORTS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP destination port: %" PRId64, dest_port);
        }
        clean_if(state->error, graph, NULL);

        int key = src;
        if (AAtree_get_exact(graph->vertices, &key) == NULL)
        {
            DSP_vertex* new_src = new_DSP_vertex(src);
            clean_if(new_src == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->vertices, new_src), graph, new_src);
        }
        DSP_vertex* src_vertex = AAtree_get_exact(graph->vertices, &key);

        key = dest;
        if (AAtree_get_exact(graph->vertices, &key) == NULL)
        {
            DSP_vertex* new_dest = new_DSP_vertex(dest);
            clean_if(new_dest == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->vertices, new_dest), graph, new_dest);
        }
        DSP_vertex* dest_vertex = AAtree_get_exact(graph->vertices, &key);

        assert(src_vertex != NULL);
        assert(dest_vertex != NULL);
        // Note: we are creating the transpose of the input graph for mixing
        //       purposes, so the edge goes from the destination to the source.
        clean_if(!DSP_vertex_set_adjacent(dest_vertex, dest_port,
                                          src_vertex, src_port), graph, NULL);

        check_next(str, state, expect_entry);
    }
    str = read_const_char(str, ']', state);
    clean_if(state->error, graph, NULL);

    if (DSP_graph_is_cyclic(graph))
    {
        Read_state_set_error(state, "The DSP graph contains a cycle");
        del_DSP_graph(graph);
        return NULL;
    }
    return graph;
}

#undef clean_if


static void DSP_graph_reset(DSP_graph* graph)
{
    assert(graph != NULL);
    int index = -1;
    DSP_vertex* vertex = AAiter_get(graph->iter, &index);
    while (vertex != NULL)
    {
        DSP_vertex_set_state(vertex, DSP_VERTEX_STATE_NEW);
        vertex = AAiter_get_next(graph->iter);
    }
    return;
}


static bool DSP_graph_is_cyclic(DSP_graph* graph)
{
    assert(graph != NULL);
    DSP_graph_reset(graph);
    int index = -1;
    DSP_vertex* vertex = AAiter_get(graph->iter, &index);
    while (vertex != NULL)
    {
        assert(DSP_vertex_get_state(vertex) != DSP_VERTEX_STATE_REACHED);
        if (DSP_vertex_cycle_in_path(vertex))
        {
            return true;
        }
        vertex = AAiter_get_next(graph->iter);
    }
    return false;
}


void del_DSP_graph(DSP_graph* graph)
{
    assert(graph != NULL);
    if (graph->iter != NULL)
    {
        del_AAiter(graph->iter);
    }
    if (graph->vertices != NULL)
    {
        del_AAtree(graph->vertices);
    }
    return;
}


