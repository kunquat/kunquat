

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Connections_search.h>
#include <Device_node.h>
#include <Generator.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef enum
{
    DEVICE_TYPE_MASTER     = 1,
    DEVICE_TYPE_GENERATOR  = 2,
    DEVICE_TYPE_EFFECT     = 4,
    DEVICE_TYPE_DSP        = 6,
    DEVICE_TYPE_INSTRUMENT = 8,
} Device_type;


typedef struct Connection
{
    Device_node* node;       ///< The neighbour node.
    int port;                ///< The port of the neighbour node.
    struct Connection* next;
} Connection;


struct Device_node
{
    char name[KQT_DEVICE_NODE_NAME_MAX];
    //Device_node* ins_dual;

    // These fields are required for adaptation to changes
    Ins_table* insts;
    DSP_table* dsps;
    Device* master; ///< The global or Instrument master

    Device_type type;
    int index;
    //Device* device;
    Device_node_state state;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


/**
 * Internal device setter.
 */
#if 0
void Device_node_set_devices_(Device_node* node,
                              Device* master,
                              Ins_table* insts,
                              Instrument* ins,
                              DSP_table* dsps);
#endif


static Device_node* Device_node_get_ins_dual(Device_node* node);


Device_node* new_Device_node(const char* name,
                             Ins_table* insts,
                             DSP_table* dsps,
                             Device* master)
{
    assert(name != NULL);
    assert(insts != NULL);
    assert(dsps != NULL);
    assert(master != NULL);
    Device_node* node = xalloc(Device_node);
    if (node == NULL)
    {
        return NULL;
    }
    strncpy(node->name, name, KQT_DEVICE_NODE_NAME_MAX - 1);
    if (string_eq(node->name, ""))
    {
        node->type = DEVICE_TYPE_MASTER;
        node->index = -1;
    }
    else if (string_has_prefix(node->name, "ins_"))
    {
        node->type = DEVICE_TYPE_INSTRUMENT;
        node->index = string_extract_index(node->name, "ins_", 2, "/");
        assert(node->index >= 0);
        assert(node->index < KQT_INSTRUMENTS_MAX);
    }
    else if (string_has_prefix(node->name, "gen_"))
    {
        node->type = DEVICE_TYPE_GENERATOR;
        node->index = string_extract_index(node->name, "gen_", 2, "/");
        assert(node->index >= 0);
        assert(node->index < KQT_GENERATORS_MAX);
    }
    else if (string_has_prefix(node->name, "dsp_"))
    {
        node->type = DEVICE_TYPE_DSP;
        node->index = string_extract_index(node->name, "dsp_", 2, "/");
        assert(node->index >= 0);
        //assert(ins != NULL || node->index < KQT_DSP_EFFECTS_MAX);
        //assert(ins == NULL || node->index < KQT_INSTRUMENT_DSPS_MAX);
    }
    else
    {
        assert(false);
    }
    //node->ins_dual = NULL;
    node->insts = insts;
    node->dsps = dsps;
    node->master = master;
    //node->device = NULL;
    node->name[KQT_DEVICE_NODE_NAME_MAX - 1] = '\0';
    node->state = DEVICE_NODE_STATE_NEW;
    node->iter = NULL;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        node->receive[port] = NULL;
        node->send[port] = NULL;
    }
    return node;
}


int Device_node_cmp(const Device_node* n1, const Device_node* n2)
{
    assert(n1 != NULL);
    assert(n2 != NULL);
    return strcmp(n1->name, n2->name);
}


void Device_node_reset(Device_node* node)
{
    assert(node != NULL);
    if (Device_node_get_state(node) == DEVICE_NODE_STATE_NEW)
    {
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_NEW);
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Instrument* ins = Ins_table_get(node->insts, node->index);
        if (ins == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Connections* ins_graph = Instrument_get_connections(ins);
        Device_node* ins_node = NULL;
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_NEW);
        node = ins_node;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_reset(edge->node);
            edge = edge->next;
        }
    }
    return;
}


#if 0
void Device_node_set_devices(Device_node* node,
                             Device* master,
                             Ins_table* insts,
                             DSP_table* dsps)
{
    assert(node != NULL);
    assert(master != NULL);
    assert(insts != NULL);
    assert(dsps != NULL);
//    fprintf(stderr, "setting...\n");
    //Device_node_set_devices_(node, master, insts, NULL, dsps);
    return;
}
#endif


#if 0
void Device_node_set_devices_(Device_node* node,
                              Device* master,
                              Ins_table* insts,
                              Instrument* ins,
                              DSP_table* dsps)
{
    assert(node != NULL);
    assert((insts != NULL) ^ (ins != NULL));
    assert(dsps != NULL);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED);
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    if (node->name[0] == '\0')
    {
        assert(master != NULL);
        node->device = master;
    }
    else if (string_has_prefix(node->name, "ins_"))
    {
        assert(insts != NULL);
        assert(ins == NULL);
        int index = string_extract_index(node->name, "ins_", 2, "/");
        assert(index >= 0);
        assert(index < KQT_INSTRUMENTS_MAX);
        ins = Ins_table_get(insts, index);
        node->device = (Device*)ins;
        if (ins != NULL)
        {
            Connections* ins_graph = Instrument_get_connections(ins);
            Device_node* ins_node = NULL;
            if (ins_graph == NULL ||
                    (ins_node = Connections_get_master(ins_graph)) == NULL)
            {
//                fprintf(stderr, "ins %p, graph %p, master %p\n",
//                                (void*)ins, (void*)ins_graph, (void*)ins_node);
                node->device = NULL;
            }
            else
            {
                Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
                Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
                ins_node->device = node->device;
                node->ins_dual = ins_node;
                node = ins_node;
                insts = NULL;
                dsps = Instrument_get_dsps(ins);
                assert(dsps != NULL);
            }
        }
    }
    else if (string_has_prefix(node->name, "gen_"))
    {
        assert(insts == NULL);
        assert(ins != NULL);
        int gen_index = string_extract_index(node->name, "gen_", 2, "/");
        assert(gen_index >= 0);
        assert(gen_index < KQT_GENERATORS_MAX);
        Generator* gen = Instrument_get_gen(ins, gen_index);
        node->device = (Device*)gen;
    }
    else if (string_has_prefix(node->name, "dsp_"))
    {
        assert(dsps != NULL);
        int index = string_extract_index(node->name, "dsp_", 2, "/");
        assert(index >= 0);
        assert(ins != NULL || index < KQT_DSP_EFFECTS_MAX);
        assert(ins == NULL || index < KQT_INSTRUMENT_DSPS_MAX);
        DSP* dsp = DSP_table_get_dsp(dsps, index);
//        if (dsp == NULL) fprintf(stderr, "null dsp is null\n");
//        else fprintf(stderr, "dsp found\n");
        node->device = (Device*)dsp;
//        fprintf(stderr, "node->device: %p\n", (void*)node->device);
    }
    else
    {
        assert(false);
    }
    if (node->device == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return;
    }
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* edge = node->receive[i];
        assert(!string_has_prefix(node->name, "gen_") || edge == NULL);
        while (edge != NULL)
        {
            Device_node_set_devices_(edge->node, NULL, insts, ins, dsps);
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}
#endif


bool Device_node_init_buffers_simple(Device_node* node)
{
    assert(node != NULL);
    assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
    if (Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED)
    {
        return true;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    if (Device_node_get_device(node) == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return true;
    }
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Instrument* ins = Ins_table_get(node->insts, node->index);
        if (ins == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Connections* ins_graph = Instrument_get_connections(ins);
        Device_node* ins_node = NULL;
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
        node = ins_node;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            assert(edge->node != NULL);
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }
            if (!Device_init_buffer(Device_node_get_device(node),
                                    DEVICE_PORT_TYPE_RECEIVE, port))
            {
                return false;
            }
#if 0
            if (node->name[0] == '\0' && node->send[0] == NULL)
            {
                fprintf(stderr, "allocated receive buffers for master\n");
                fprintf(stderr, "%p %p   \n",
                        (void*)Audio_buffer_get_buffer(Device_get_buffer(node->device, DEVICE_PORT_TYPE_RECEIVE, port), 0),
                        (void*)Audio_buffer_get_buffer(Device_get_buffer(node->device, DEVICE_PORT_TYPE_RECEIVE, port), 1));
            }
#endif
            if (!Device_init_buffer(Device_node_get_device(edge->node),
                                    DEVICE_PORT_TYPE_SEND, edge->port))
            {
                return false;
            }
            if (!Device_node_init_buffers_simple(edge->node))
            {
                return false;
            }
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return true;
}


void Device_node_remove_direct_buffers(Device_node* node)
{
    assert(node != NULL);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    if (Device_node_get_device(node) == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return;
    }
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Device_node* ins_node = Device_node_get_ins_dual(node);
        if (ins_node == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
        node = ins_node;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_remove_direct_buffers(edge->node);
            edge = edge->next;
        }
    }
    Device_remove_direct_buffers(Device_node_get_device(node));
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


bool Device_node_init_input_buffers(Device_node* node)
{
    assert(node != NULL);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
        return true;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    if (Device_node_get_device(node) == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return true;
    }
    bool instrument_master = false;
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Instrument* ins = Ins_table_get(node->insts, node->index);
        if (ins == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Connections* ins_graph = Instrument_get_connections(ins);
        Device_node* ins_node = NULL;
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
        node = ins_node;
        instrument_master = true;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }
            if (!instrument_master &&
                    !Device_init_buffer(Device_node_get_device(node),
                                          DEVICE_PORT_TYPE_RECEIVE, port))
            {
                return false;
            }
            if (!Device_node_init_input_buffers(edge->node))
            {
                return false;
            }
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return true;
}


bool Device_node_init_buffers_by_suggestion(Device_node* node,
                                            int send_port,
                                            Audio_buffer* suggestion)
{
    assert(node != NULL);
    assert(send_port >= 0);
    assert(send_port < KQT_DEVICE_PORTS_MAX);
    Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return true;
    }
    assert(node->name[0] == '\0' || node->send[send_port] != NULL);
    if (node->type == DEVICE_TYPE_INSTRUMENT && // node->ins_dual != NULL &&
            Device_get_buffer(node_device, DEVICE_PORT_TYPE_RECEIVE, send_port) ==
                Device_get_buffer(node_device, DEVICE_PORT_TYPE_SEND, send_port) &&
            Device_get_buffer(node_device, DEVICE_PORT_TYPE_SEND, send_port) != NULL)
    {
        return true;
    }
    else if (node->type != DEVICE_TYPE_INSTRUMENT && // node->ins_dual == NULL &&
            Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED);
        return true;
    }
//    if (Device_get_buffer(node->device, DEVICE_PORT_TYPE_SEND, send_port)
//            != NULL)
//    {
//        return true;
//    }
    if (suggestion != NULL && node->send[send_port]->next == NULL)
    {
        Device_set_direct_send(node_device, send_port, suggestion);
    }
    else if (!Device_init_buffer(node_device,
                                 DEVICE_PORT_TYPE_SEND,
                                 send_port))
    {
        return false;
    }
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Instrument* ins = Ins_table_get(node->insts, node->index);
        if (ins == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Connections* ins_graph = Instrument_get_connections(ins);
        Device_node* ins_node = NULL;
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }
        Device_set_direct_receive(node_device, send_port);
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
        node = ins_node;
        node_device = Device_node_get_device(node);
        Connection* edge = node->receive[send_port];
        suggestion = Device_get_buffer(node_device,
                                       DEVICE_PORT_TYPE_RECEIVE,
                                       send_port);
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }
            if (!Device_node_init_buffers_by_suggestion(edge->node,
                                                        edge->port,
                                                        suggestion))
            {
                return false;
            }
            edge = edge->next;
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return true;
    }
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
        return true;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        suggestion = Device_get_buffer(node_device,
                                       DEVICE_PORT_TYPE_RECEIVE, port);
        if (suggestion == NULL)
        {
            continue;
        }
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }
            if (!Device_node_init_buffers_by_suggestion(edge->node,
                        edge->port, suggestion))
            {
                return false;
            }
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return true;
}


void Device_node_clear_buffers(Device_node* node,
                               uint32_t start,
                               uint32_t until)
{
    assert(node != NULL);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED);
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    if (Device_node_get_device(node) == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return;
    }
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Device_node* ins_node = Device_node_get_ins_dual(node);
        if (ins_node == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        Device_node_set_state(ins_node, DEVICE_NODE_STATE_REACHED);
        node = ins_node;
    }
    //fprintf(stderr, "Clearing buffers of %p\n", (void*)Device_node_get_device(node));
    Device_clear_buffers(Device_node_get_device(node), start, until);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_clear_buffers(edge->node, start, until);
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


void Device_node_mix(Device_node* node,
                     uint32_t start,
                     uint32_t until,
                     uint32_t freq,
                     double tempo)
{
    assert(node != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED);
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return;
    }
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Instrument* ins = Ins_table_get(node->insts, node->index);
        if (ins == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Connections* ins_graph = Instrument_get_connections(ins);
        Device_node* ins_node = NULL;
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }
        Device_node_mix(ins_node, start, until, freq, tempo);
        for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
        {
            Audio_buffer* receive = Device_get_buffer(node_device,
                                            DEVICE_PORT_TYPE_RECEIVE, port);
            Audio_buffer* send = Device_get_buffer(node_device,
                                            DEVICE_PORT_TYPE_SEND, port);
            if (receive == NULL || send == NULL)
            {
                continue;
            }
            Audio_buffer_mix(send, receive, start, until);
        }
        Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
        return;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }
            Device_node_mix(edge->node, start, until, freq, tempo);
            Audio_buffer* send =
                    Device_get_buffer(Device_node_get_device(edge->node),
                                         DEVICE_PORT_TYPE_SEND, edge->port);
            Audio_buffer* receive = Device_get_buffer(node_device,
                                         DEVICE_PORT_TYPE_RECEIVE, port);
            if (receive == NULL || send == NULL)
            {
#if 0
                if (receive != NULL)
                {
                    fprintf(stderr, "receive %p, but no send!\n", (void*)receive);
                }
                else if (send != NULL)
                {
                    fprintf(stderr, "send %p, but no receive!\n", (void*)send);
                }
#endif
                edge = edge->next;
                continue;
            }
            Audio_buffer_mix(receive, send, start, until);
            edge = edge->next;
        }
    }
    Device_process(node_device, start, until, freq, tempo);
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


char* Device_node_get_name(Device_node* node)
{
    assert(node != NULL);
    return node->name;
}


Device* Device_node_get_device(Device_node* node)
{
    assert(node != NULL);
    if (node->type == DEVICE_TYPE_MASTER)
    {
        return node->master;
    }
    else if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        return (Device*)Ins_table_get(node->insts, node->index);
    }
    else if (node->type == DEVICE_TYPE_GENERATOR)
    {
        return (Device*)Instrument_get_gen((Instrument*)node->master,
                                           node->index);
    }
    else if (node->type == DEVICE_TYPE_DSP)
    {
        return (Device*)DSP_table_get_dsp(node->dsps, node->index);
    }
    assert(false);
    return NULL;
}


void Device_node_set_state(Device_node* node, Device_node_state state)
{
    assert(node != NULL);
//    assert(state >= DEVICE_NODE_STATE_NEW);
    assert(state <= DEVICE_NODE_STATE_VISITED);
    node->state = state;
    return;
}


Device_node_state Device_node_get_state(Device_node* node)
{
    assert(node != NULL);
    return node->state;
}


bool Device_node_connect(Device_node* receiver,
                         int rec_port,
                         Device_node* sender,
                         int send_port)
{
    assert(receiver != NULL);
    assert(rec_port >= 0);
    assert(rec_port < KQT_DEVICE_PORTS_MAX);
    assert(sender != NULL);
    assert(send_port >= 0);
    assert(send_port < KQT_DEVICE_PORTS_MAX);
    Connection* receive_edge = xalloc(Connection);
    if (receive_edge == NULL)
    {
        return false;
    }
    Connection* send_edge = xalloc(Connection);
    if (send_edge == NULL)
    {
        xfree(receive_edge);
        return false;
    }

    receive_edge->node = sender;
    receive_edge->port = send_port;
    receive_edge->next = receiver->receive[rec_port];
    receiver->receive[rec_port] = receive_edge;

    send_edge->node = receiver;
    send_edge->port = rec_port;
    send_edge->next = sender->send[send_port];
    sender->send[send_port] = send_edge;
    return true;
}


#if 0
void Device_node_disconnect(Device_node* node, Device* device)
{
    assert(node != NULL);
    assert(device != NULL);
    // NOTE: device must not be dereferenced here
    //       because it may already be destroyed.
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* prev = NULL;
        Connection* cur = node->receive[i];
        while (cur != NULL)
        {
            Connection* next = cur->next;
            assert(cur->node != NULL);
            if (cur->node->device == device)
            {
                if (prev == NULL)
                {
                    node->receive[i] = next;
                    xfree(cur);
                    cur = NULL;
                }
                else
                {
                    prev->next = next;
                    xfree(cur);
                    cur = prev;
                }
            }
            prev = cur;
            cur = next;
        }
        prev = NULL;
        cur = node->send[i];
        while (cur != NULL)
        {
            Connection* next = cur->next;
            assert(cur->node != NULL);
            if (cur->node->device == device)
            {
                if (prev == NULL)
                {
                    node->send[i] = next;
                    xfree(cur);
                    cur = NULL;
                }
                else
                {
                    prev->next = next;
                    xfree(cur);
                    cur = prev;
                }
            }
            prev = cur;
            cur = next;
        }
    }
    return;
}
#endif


#if 0
void Device_node_replace(Device_node* node,
                         Device* old_device,
                         Device* new_device)
{
    assert(node != NULL);
    assert(old_device != NULL);
    assert(new_device != NULL);
    assert(new_device != old_device);
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* cur = node->receive[i];
        while (cur != NULL)
        {
            assert(cur->node->device != new_device);
            if (cur->node->device == old_device)
            {
                cur->node->device = new_device;
            }
            cur = cur->next;
        }
        cur = node->send[i];
        while (cur != NULL)
        {
            assert(cur->node->device != new_device);
            if (cur->node->device == old_device)
            {
                cur->node->device = new_device;
            }
            cur = cur->next;
        }
    }
    return;
}
#endif


Device_node* Device_node_get_sender(Device_node* node,
                                    int rec_port,
                                    int* send_port)
{
    assert(node != NULL);
    assert(rec_port >= 0);
    assert(rec_port < KQT_DEVICE_PORTS_MAX);
    node->iter = node->receive[rec_port];
    if (node->iter == NULL)
    {
        return NULL;
    }
    if (send_port != NULL)
    {
        *send_port = node->iter->port;
    }
    return node->iter->node;
}


Device_node* Device_node_get_receiver(Device_node* node,
                                      int send_port,
                                      int* rec_port)
{
    assert(node != NULL);
    assert(send_port >= 0);
    assert(send_port < KQT_DEVICE_PORTS_MAX);
    node->iter = node->send[send_port];
    if (node->iter == NULL)
    {
        return NULL;
    }
    if (rec_port != NULL)
    {
        *rec_port = node->iter->port;
    }
    return node->iter->node;
}


Device_node* Device_node_get_next(Device_node* node, int* port)
{
    assert(node != NULL);
    if (node->iter == NULL)
    {
        return NULL;
    }
    node->iter = node->iter->next;
    if (node->iter != NULL && port != NULL)
    {
        *port = node->iter->port;
    }
    return node->iter->node;
}


bool Device_node_cycle_in_path(Device_node* node)
{
    assert(node != NULL);
    if (Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED)
    {
        return false;
    }
    if (Device_node_get_state(node) == DEVICE_NODE_STATE_REACHED)
    {
        return true;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* edge = node->receive[i];
        while (edge != NULL)
        {
            if (Device_node_cycle_in_path(edge->node))
            {
                return true;
            }
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return false;
}


void Device_node_print(Device_node* node, FILE* out)
{
    assert(node != NULL);
    assert(out != NULL);
    const char* states[] =
    {
        [DEVICE_NODE_STATE_NEW] = "new",
        [DEVICE_NODE_STATE_REACHED] = "reached",
        [DEVICE_NODE_STATE_VISITED] = "visited",
    };
    fprintf(out, "\nDevice node: %s, Device: %p, (state: %s)\n",
                 node->name[0] == '\0' ? "[Master]" : node->name,
                 (void*)Device_node_get_device(node),
                 states[Device_node_get_state(node)]);
    if (node->type == DEVICE_TYPE_INSTRUMENT)
    {
        Device_node* ins_node = Device_node_get_ins_dual(node);
        if (ins_node == NULL)
        {
            return;
        }
        fprintf(out, "Instrument dual:");
        Device_node_print(ins_node, out);
        return;
    }
    if (Device_node_get_device(node) != NULL)
    {
        Device_print(Device_node_get_device(node), out);
    }
    bool conn_printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        bool port_printed = false;
        while (edge != NULL)
        {
            if (!conn_printed)
            {
                fprintf(out, "Connections to receive ports:\n");
                conn_printed = true;
            }
            if (!port_printed)
            {
                fprintf(out, "  Port %02x:\n", port);
                port_printed = true;
            }
            fprintf(out, "    %s (device %p), send port %02x\n",
                         edge->node->name,
                         (void*)Device_node_get_device(edge->node),
                         edge->port);
            edge = edge->next;
        }
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            assert(edge->node->send[edge->port] != NULL);
            if (node == edge->node->send[edge->port]->node)
            {
                Device_node_print(edge->node, out);
            }
            edge = edge->next;
        }
    }
    return;
}


void del_Device_node(Device_node* node)
{
    if (node == NULL)
    {
        return;
    }
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* cur = node->receive[i];
        Connection* next = NULL;
        while (cur != NULL)
        {
            next = cur->next;
            xfree(cur);
            cur = next;
        }
        cur = node->send[i];
        next = NULL;
        while (cur != NULL)
        {
            next = cur->next;
            xfree(cur);
            cur = next;
        }
    }
    xfree(node);
    return;
}


static Device_node* Device_node_get_ins_dual(Device_node* node)
{
    assert(node != NULL);
    assert(node->type == DEVICE_TYPE_INSTRUMENT);
    Instrument* ins = Ins_table_get(node->insts, node->index);
    if (ins == NULL)
    {
        return NULL;
    }
    Connections* ins_graph = Instrument_get_connections(ins);
    if (ins_graph == NULL)
    {
        return NULL;
    }
    return Connections_get_master(ins_graph);
}


