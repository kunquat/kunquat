

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
#include <Connections.h>
#include <Device_node.h>
#include <string_common.h>

#include <xmemory.h>


struct Connections
{
    AAtree* nodes;
    AAiter* iter;
};


/**
 * Resets the graph for searching purposes.
 *
 * \param graph   The Connections -- must not be \c NULL.
 */
static void Connections_reset(Connections* graph);


/**
 * Tells whether there is a cycle inside Connections.
 *
 * All Connections must be acyclic.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   \c true if there is a cycle in \a graph, otherwise \c false.
 */
static bool Connections_is_cyclic(Connections* graph);


/**
 * Validates a connection path.
 *
 * This function also strips the port directory off the path.
 *
 * \param str         The path -- must not be \c NULL.
 * \param ins_level   Whether this connection is in the Instrument level or not.
 * \param state       The Read state -- must not be \c NULL.
 *
 * \return   The port number if the path is valid, otherwise \c -1.
 */
static int validate_connection_path(char* str,
                                    bool ins_level,
                                    Read_state* state);


#define clean_if(expr, graph, node)   \
    if (true)                           \
    {                                   \
        if ((expr))                     \
        {                               \
            if (node != NULL)         \
            {                           \
                del_Device_node(node); \
            }                           \
            del_Connections(graph);     \
            return NULL;                \
        }                               \
    } else (void)0

Connections* new_Connections_from_string(char* str,
                                         bool ins_level,
                                         Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Connections* graph = xalloc(Connections);
    if (graph == NULL)
    {
        return NULL;
    }
    graph->nodes = NULL;
    graph->iter = NULL;
    graph->nodes = new_AAtree((int (*)(const void*, const void*))Device_node_cmp,
                                 (void (*)(void*))del_Device_node);
    clean_if(graph->nodes == NULL, graph, NULL);
    graph->iter = new_AAiter(graph->nodes);
    clean_if(graph->iter == NULL, graph, NULL);

    Device_node* master = new_Device_node("/");
    clean_if(master == NULL, graph, NULL);
    clean_if(!AAtree_ins(graph->nodes, master), graph, master);

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
        char src_name[KQT_DEVICE_NODE_NAME_MAX] = { '\0' };
        str = read_string(str, src_name, KQT_DEVICE_NODE_NAME_MAX, state);
        str = read_const_char(str, ',', state);
        char dest_name[KQT_DEVICE_NODE_NAME_MAX] = { '\0' };
        str = read_string(str, dest_name, KQT_DEVICE_NODE_NAME_MAX, state);
        str = read_const_char(str, ']', state);

        int src_port = validate_connection_path(src_name, ins_level, state);
        int dest_port = validate_connection_path(dest_name, ins_level, state);
        clean_if(state->error, graph, NULL);
        
        if (AAtree_get_exact(graph->nodes, src_name) == NULL)
        {
            Device_node* new_src = new_Device_node(src_name);
            clean_if(new_src == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->nodes, new_src), graph, new_src);
        }
        Device_node* src_node = AAtree_get_exact(graph->nodes, src_name);

        if (AAtree_get_exact(graph->nodes, dest_name) == NULL)
        {
            Device_node* new_dest = new_Device_node(dest_name);
            clean_if(new_dest == NULL, graph, NULL);
            clean_if(!AAtree_ins(graph->nodes, new_dest), graph, new_dest);
        }
        Device_node* dest_node = AAtree_get_exact(graph->nodes, dest_name);

        assert(src_node != NULL);
        assert(dest_node != NULL);
        clean_if(!Device_node_connect(dest_node, dest_port,
                                     src_node, src_port), graph, NULL);

        check_next(str, state, expect_entry);
    }
    str = read_const_char(str, ']', state);
    clean_if(state->error, graph, NULL);

    if (Connections_is_cyclic(graph))
    {
        Read_state_set_error(state, "The connection graph contains a cycle");
        del_Connections(graph);
        return NULL;
    }
    return graph;
}

#undef clean_if


void Connections_set_devices(Connections* graph,
                             Device* master,
                             Ins_table* insts /*,
                             DSP_table* dsps */)
{
    assert(graph != NULL);
    assert(master != NULL);
    assert(insts != NULL);
//    assert(dsps != NULL);
    Connections_reset(graph);
    Device_node* master_node = AAtree_get_exact(graph->nodes, "/");
    assert(master_node != NULL);
    Device_node_set_devices(master_node, master, insts/*, dsps*/);
    return;
}


static void Connections_reset(Connections* graph)
{
    assert(graph != NULL);
    const char* name = "";
    Device_node* node = AAiter_get(graph->iter, name);
    while (node != NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_NEW);
        node = AAiter_get_next(graph->iter);
    }
    return;
}


static bool Connections_is_cyclic(Connections* graph)
{
    assert(graph != NULL);
    Connections_reset(graph);
    const char* name = "";
    Device_node* node = AAiter_get(graph->iter, name);
    while (node != NULL)
    {
        assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
        if (Device_node_cycle_in_path(node))
        {
            return true;
        }
        node = AAiter_get_next(graph->iter);
    }
    return false;
}


void del_Connections(Connections* graph)
{
    assert(graph != NULL);
    if (graph->iter != NULL)
    {
        del_AAiter(graph->iter);
    }
    if (graph->nodes != NULL)
    {
        del_AAtree(graph->nodes);
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


static int validate_connection_path(char* str,
                                    bool ins_level,
                                    Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return -1;
    }
    bool instrument = false;
    bool generator = false;
    bool dsp = false;
    bool root = true;
    char* trim_point = str;
    if (string_has_prefix(str, "ins_"))
    {
        if (ins_level)
        {
            Read_state_set_error(state,
                    "Instrument directory in an instrument-level connection");
            return -1;
        }
        instrument = true;
        root = false;
        str += strlen("ins_");
        if (read_index(str) >= KQT_INSTRUMENTS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid instrument number in the connection");
            return -1;
        }
        str += 2;
        if (!string_has_prefix(str, "/" MAGIC_ID "iXX/"))
        {
            Read_state_set_error(state,
                    "Missing instrument header after the instrument number"
                    " in the connection");
            return -1;
        }
        str += strlen("/" MAGIC_ID "iXX/");
        trim_point = str - 1;
    }
    else if (string_has_prefix(str, "gen_"))
    {
        if (!ins_level)
        {
            Read_state_set_error(state,
                    "Generator directory in a root-level connection");
        }
        root = false;
        generator = true;
        str += strlen("gen_");
        if (read_index(str) >= KQT_GENERATORS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid generator number in the connection");
            return -1;
        }
        str += 2;
        if (!string_has_prefix(str, "/" MAGIC_ID "gXX/"))
        {
            Read_state_set_error(state,
                    "Missing generator header after the generator number"
                    " in the connection");
            return -1;
        }
        str += strlen("/" MAGIC_ID "gXX/");
        if (!string_has_prefix(str, "C/"))
        {
            Read_state_set_error(state,
                    "Invalid generator parameter directory"
                    " in the connection");
            return -1;
        }
        str += strlen("C/");
        trim_point = str - 1;
    }
    else if (string_has_prefix(str, "dsp_"))
    {
        root = false;
        dsp = true;
        str += strlen("dsp_");
        if (read_index(str) >= KQT_DSP_EFFECTS_MAX)
        {
            Read_state_set_error(state,
                    "Invalid DSP number in the connection");
            return -1;
        }
        str += 2;
        if (!string_has_prefix(str, "/" MAGIC_ID "eXX/"))
        {
            Read_state_set_error(state,
                    "Missing DSP header after the DSP number"
                    " in the connection", *str);
            return -1;
        }
        str += strlen("/" MAGIC_ID "eXX/");
        if (!string_has_prefix(str, "C/"))
        {
            Read_state_set_error(state,
                    "Invalid DSP parameter directory"
                    " in the connection");
            return -1;
        }
        str += strlen("C/");
        trim_point = str - 1;
    }
    if (string_has_prefix(str, "in_") || string_has_suffix(str, "out_"))
    {
        if (string_has_prefix(str, "in_") && (instrument || generator))
        {
            Read_state_set_error(state,
                    "Input ports are not allowed for instruments"
                    " or generators");
            return -1;
        }
        if (string_has_prefix(str, "in_") && root)
        {
            Read_state_set_error(state,
                    "Input ports are not allowed for root");
        }
        str += strcspn(str, "_") + 1;
        int port = read_index(str);
        if (port >= KQT_DEVICE_PORTS_MAX)
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


