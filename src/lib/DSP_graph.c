

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
#include <limits.h>
#include <string.h>

#include <AAtree.h>
#include <DSP_graph.h>
#include <DSP_vertex.h>
#include <string_common.h>

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


/**
 * Validates a connection path.
 *
 * This function also strips the port directory off the path.
 *
 * \param str     The path -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The port number if the path is valid, otherwise \c -1.
 */
static int validate_connection_path(char* str, Read_state* state);


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

    DSP_vertex* master = new_DSP_vertex("/");
    clean_if(master == NULL, graph, NULL);
    clean_if(!AAtree_ins(graph->vertices, master), graph, master);

    if (str == NULL)
    {
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
        char src_name[KQT_DSP_VERTEX_NAME_MAX] = { '\0' };
        str = read_string(str, src_name, KQT_DSP_VERTEX_NAME_MAX, state);
        str = read_const_char(str, ',', state);
        char dest_name[KQT_DSP_VERTEX_NAME_MAX] = { '\0' };
        str = read_string(str, dest_name, KQT_DSP_VERTEX_NAME_MAX, state);
        str = read_const_char(str, ']', state);

        int src_port = validate_connection_path(src_name, state);
        int dest_port = validate_connection_path(dest_name, state);
        clean_if(state->error, graph, NULL);
        
        if (AAtree_get_exact(graph->vertices, src_name) == NULL)
        {
            DSP_vertex* new_src = new_DSP_vertex(src_name);
            clean_if(new_src == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->vertices, new_src), graph, new_src);
        }
        DSP_vertex* src_vertex = AAtree_get_exact(graph->vertices, src_name);

        if (AAtree_get_exact(graph->vertices, dest_name) == NULL)
        {
            DSP_vertex* new_dest = new_DSP_vertex(dest_name);
            clean_if(new_dest == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->vertices, new_dest), graph, new_dest);
        }
        DSP_vertex* dest_vertex = AAtree_get_exact(graph->vertices, dest_name);

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


static int read_index(char* str)
{
    assert(str != NULL);
    static const char* hex_digits = "0123456789abcdef";
    if (strspn(str, hex_digits) != 2)
    {
        return INT_MAX;
    }
    int res = (strchr(hex_digits, str[0]) - hex_digits) * 0x10;
    return res + (strchr(hex_digits, str[1]) - hex_digits);
}


static int validate_connection_path(char* str, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return -1;
    }
    if (*str != '/')
    {
        Read_state_set_error(state, "The connection begins with '%c'"
                " instead of '/'", *str);
        return -1;
    }
    ++str;
    bool in_allowed = false;
    bool generator = false;
    if (string_has_prefix(str, "ins_"))
    {
        str += strlen("ins_");
        if (read_index(str) >= KQT_INSTRUMENTS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid instrument number in the connection");
            return -1;
        }
        str += 2;
        if (*str != '/')
        {
            Read_state_set_error(state,
                    "Unexpected '%c' after the instrument number"
                    " in the connection", *str);
            return -1;
        }
        ++str;
        if (!string_has_prefix(str, MAGIC_ID "iXX/"))
        {
            Read_state_set_error(state, "Invalid instrument header"
                    " in the connection");
            return -1;
        }
        str += strlen(MAGIC_ID "iXX/");
        if (string_has_prefix(str, "gen_"))
        {
            generator = true;
            str += strlen("gen_");
            if (read_index(str) >= KQT_GENERATORS_MAX)
            {
                Read_state_set_error(state,
                        "Invalid generator number in the connection");
                return -1;
            }
            str += 2;
            if (*str != '/')
            {
                Read_state_set_error(state,
                        "Unexpected '%c' after the generator number"
                        " in the connection", *str);
                return -1;
            }
            ++str;
            if (!string_has_prefix(str, "C/"))
            {
                Read_state_set_error(state,
                        "Invalid generator parameter directory"
                        " in the connection");
                return -1;
            }
            str += strlen("C/");
        }
    }
    if (string_has_prefix(str, "dsp_") && !generator)
    {
        in_allowed = true;
        str += strlen("dsp_");
        if (read_index(str) >= KQT_DSP_NODES_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP number in the connection");
            return -1;
        }
        str += 2;
        if (*str != '/')
        {
            Read_state_set_error(state,
                    "Unexpected '%c' after the DSP number"
                    " in the connection", *str);
            return -1;
        }
        ++str;
        if (!string_has_prefix(str, "C/"))
        {
            Read_state_set_error(state,
                    "Invalid DSP parameter directory"
                    " in the connection");
            return -1;
        }
        str += strlen("C/");
    }
    if (string_has_prefix(str, "in_") || string_has_suffix(str, "out_"))
    {
        if (string_has_prefix(str, "in_") && !in_allowed)
        {
            Read_state_set_error(state,
                    "Input ports are not allowed for instruments"
                    " or generators");
            return -1;
        }
        char* trim_point = str;
        str += strcspn(str, "_") + 1;
        int port = read_index(str);
        if (port >= KQT_DSP_PORTS_MAX)
        {
            Read_state_set_error(state, "Invalid port number");
            return -1;
        }
        str += 2;
        if (str[0] != '/' && str[0] != '\0' && str[1] != '\0')
        {
            Read_state_set_error(state, "Connection path contains garbage"
                    " after the port specification");
            return -1;
        }
        *trim_point = '\0';
        return port;
    }
    Read_state_set_error(state, "Invalid connection");
    return -1;
}


