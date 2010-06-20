

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
#include <string.h>

#include <DSP_vertex.h>
#include <kunquat/limits.h>

#include <xmemory.h>


typedef struct DSP_edge
{
    DSP_vertex* vertex; ///< The neighbour node.
    int out_port;       ///< The output port of the neighbour node.
    struct DSP_edge* next;
} DSP_edge;


struct DSP_vertex
{
    char name[KQT_DSP_VERTEX_NAME_MAX];
    DSP_vertex_state state;
    DSP_edge* iter;
    DSP_edge* adj[KQT_DSP_PORTS_MAX];
};


DSP_vertex* new_DSP_vertex(const char* name)
{
    assert(name != NULL);
    DSP_vertex* vertex = xalloc(DSP_vertex);
    if (vertex == NULL)
    {
        return NULL;
    }
    strncpy(vertex->name, name, KQT_DSP_VERTEX_NAME_MAX - 1);
    vertex->name[KQT_DSP_VERTEX_NAME_MAX - 1] = '\0';
    vertex->state = DSP_VERTEX_STATE_NEW;
    vertex->iter = NULL;
    for (int i = 0; i < KQT_DSP_PORTS_MAX; ++i)
    {
        vertex->adj[i] = NULL;
    }
    return vertex;
}


int DSP_vertex_cmp(const DSP_vertex* v1, const DSP_vertex* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);
    return strcmp(v1->name, v2->name);
}


char* DSP_vertex_get_name(DSP_vertex* vertex)
{
    assert(vertex != NULL);
    return vertex->name;
}


void DSP_vertex_set_state(DSP_vertex* vertex, DSP_vertex_state state)
{
    assert(vertex != NULL);
//    assert(state >= DSP_VERTEX_STATE_NEW);
    assert(state <= DSP_VERTEX_STATE_VISITED);
    vertex->state = state;
    return;
}


DSP_vertex_state DSP_vertex_get_state(DSP_vertex* vertex)
{
    assert(vertex != NULL);
    return vertex->state;
}


bool DSP_vertex_set_adjacent(DSP_vertex* vertex,
                             int in_port,
                             DSP_vertex* adj,
                             int out_port)
{
    assert(vertex != NULL);
    assert(in_port >= 0);
    assert(in_port < KQT_DSP_PORTS_MAX);
    assert(adj != NULL);
    assert(out_port >= 0);
    assert(out_port < KQT_DSP_PORTS_MAX);
    DSP_edge* edge = xalloc(DSP_edge);
    if (edge == NULL)
    {
        return false;
    }
    edge->vertex = adj;
    edge->out_port = out_port;
    edge->next = vertex->adj[in_port];
    vertex->adj[in_port] = edge;
    return true;
}


DSP_vertex* DSP_vertex_get_adjacent(DSP_vertex* vertex,
                                    int in_port,
                                    int* out_port)
{
    assert(vertex != NULL);
    assert(in_port >= 0);
    assert(in_port < KQT_DSP_PORTS_MAX);
    vertex->iter = vertex->adj[in_port];
    if (vertex->iter == NULL)
    {
        return NULL;
    }
    if (out_port != NULL)
    {
        *out_port = vertex->iter->out_port;
    }
    return vertex->iter->vertex;
}


DSP_vertex* DSP_vertex_get_next(DSP_vertex* vertex, int* out_port)
{
    assert(vertex != NULL);
    if (vertex->iter == NULL)
    {
        return NULL;
    }
    vertex->iter = vertex->iter->next;
    if (vertex->iter != NULL && out_port != NULL)
    {
        *out_port = vertex->iter->out_port;
    }
    return vertex->iter->vertex;
}


bool DSP_vertex_cycle_in_path(DSP_vertex* vertex)
{
    assert(vertex != NULL);
    if (DSP_vertex_get_state(vertex) == DSP_VERTEX_STATE_VISITED)
    {
        return false;
    }
    if (DSP_vertex_get_state(vertex) == DSP_VERTEX_STATE_REACHED)
    {
        return true;
    }
    DSP_vertex_set_state(vertex, DSP_VERTEX_STATE_REACHED);
    for (int i = 0; i < KQT_DSP_PORTS_MAX; ++i)
    {
        DSP_edge* edge = vertex->adj[i];
        while (edge != NULL)
        {
            if (DSP_vertex_cycle_in_path(edge->vertex))
            {
                return true;
            }
            edge = edge->next;
        }
    }
    DSP_vertex_set_state(vertex, DSP_VERTEX_STATE_VISITED);
    return false;
}


void del_DSP_vertex(DSP_vertex* vertex)
{
    assert(vertex != NULL);
    for (int i = 0; i < KQT_DSP_PORTS_MAX; ++i)
    {
        DSP_edge* cur = vertex->adj[i];
        DSP_edge* next = NULL;
        while (cur != NULL)
        {
            next = cur->next;
            xfree(cur);
            cur = next;
        }
    }
    xfree(vertex);
    return;
}


