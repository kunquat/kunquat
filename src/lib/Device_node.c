

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
#include <string.h>

#include <Connections_search.h>
#include <Device_node.h>
#include <Generator.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct Connection
{
    Device_node* node;       ///< The neighbour node.
    int port;                ///< The port of the neighbour node.
    struct Connection* next;
} Connection;


struct Device_node
{
    char name[KQT_DEVICE_NODE_NAME_MAX];
    Device_node* ins_dual;
    Device* device;
    Device_node_state state;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


/**
 * Internal device setter.
 */
void Device_node_set_devices_(Device_node* node,
                              Device* master,
                              Ins_table* insts,
                              Instrument* ins/*,
                              DSP_table* dsps*/);


Device_node* new_Device_node(const char* name)
{
    assert(name != NULL);
    Device_node* node = xalloc(Device_node);
    if (node == NULL)
    {
        return NULL;
    }
    strncpy(node->name, name, KQT_DEVICE_NODE_NAME_MAX - 1);
    node->ins_dual = NULL;
    node->device = NULL;
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
    if (node->ins_dual != NULL)
    {
        Device_node_set_state(node->ins_dual, DEVICE_NODE_STATE_NEW);
        node = node->ins_dual;
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


void Device_node_set_devices(Device_node* node,
                             Device* master,
                             Ins_table* insts/*,
                             DSP_table* dsps*/)
{
    assert(node != NULL);
    assert(master != NULL);
    assert(insts != NULL);
//    assert(dsps != NULL);
//    fprintf(stderr, "setting...\n");
    Device_node_set_devices_(node, master, insts, NULL/*, dsps*/);
    return;
}


void Device_node_set_devices_(Device_node* node,
                              Device* master,
                              Ins_table* insts,
                              Instrument* ins/*,
                              DSP_table* dsps*/)
{
    assert(node != NULL);
    assert((insts != NULL) ^ (ins != NULL));
//    assert(dsps != NULL);
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
#if 0
        const char* num_s = strchr(node->name, '_');
        assert(num_s != NULL);
        ++num_s;
        char* num_s_end = NULL;
        long index = strtol(num_s, &num_s_end, 16);
        assert(num_s + 2 == num_s_end);
#endif
        int index = string_extract_index(node->name, "ins_", 2);
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
                ins_node->device = node->device;
                node->ins_dual = ins_node;
                node = ins_node;
                insts = NULL;
            }
        }
    }
    else if (string_has_prefix(node->name, "gen_"))
    {
        assert(insts == NULL);
        assert(ins != NULL);
        int gen_index = string_extract_index(node->name, "gen_", 2);
        assert(gen_index >= 0);
        assert(gen_index < KQT_GENERATORS_MAX);
        Generator* gen = Instrument_get_gen(ins, gen_index);
        node->device = (Device*)gen;
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
            Device_node_set_devices_(edge->node, NULL, insts, ins/*, dsps*/);
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


char* Device_node_get_name(Device_node* node)
{
    assert(node != NULL);
    return node->name;
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
    assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
    assert(out != NULL);
    if (Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED)
    {
        return;
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    fprintf(out, "\nDevice node: %s, Device: %p\n",
                 node->name[0] == '\0' ? "[Master]" : node->name,
                 (void*)node->device);
    if (node->ins_dual != NULL)
    {
        node = node->ins_dual;
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
                fprintf(out, "Connections to input ports:\n");
                conn_printed = true;
            }
            if (!port_printed)
            {
                fprintf(out, "  Port %d:\n", port);
                port_printed = true;
            }
            fprintf(out, "    %s\n", edge->node->name);
            edge = edge->next;
        }
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_print(edge->node, out);
            edge = edge->next;
        }
    }
    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


void del_Device_node(Device_node* node)
{
    assert(node != NULL);
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


