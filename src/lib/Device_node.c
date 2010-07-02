

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

#include <Device_node.h>
#include <kunquat/limits.h>

#include <xmemory.h>


typedef struct Connection
{
    Device_node* vertex;     ///< The neighbour node.
    int port;                ///< The port of the neighbour node.
    struct Connection* next;
} Connection;


struct Device_node
{
    char name[KQT_DEVICE_NODE_NAME_MAX];
    Device_node_state state;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


Device_node* new_Device_node(const char* name)
{
    assert(name != NULL);
    Device_node* vertex = xalloc(Device_node);
    if (vertex == NULL)
    {
        return NULL;
    }
    strncpy(vertex->name, name, KQT_DEVICE_NODE_NAME_MAX - 1);
    vertex->name[KQT_DEVICE_NODE_NAME_MAX - 1] = '\0';
    vertex->state = DEVICE_NODE_STATE_NEW;
    vertex->iter = NULL;
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        vertex->receive[i] = NULL;
        vertex->send[i] = NULL;
    }
    return vertex;
}


int Device_node_cmp(const Device_node* v1, const Device_node* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);
    return strcmp(v1->name, v2->name);
}


char* Device_node_get_name(Device_node* vertex)
{
    assert(vertex != NULL);
    return vertex->name;
}


void Device_node_set_state(Device_node* vertex, Device_node_state state)
{
    assert(vertex != NULL);
//    assert(state >= DEVICE_NODE_STATE_NEW);
    assert(state <= DEVICE_NODE_STATE_VISITED);
    vertex->state = state;
    return;
}


Device_node_state Device_node_get_state(Device_node* vertex)
{
    assert(vertex != NULL);
    return vertex->state;
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

    receive_edge->vertex = sender;
    receive_edge->port = send_port;
    receive_edge->next = receiver->receive[rec_port];
    receiver->receive[rec_port] = receive_edge;

    send_edge->vertex = receiver;
    send_edge->port = rec_port;
    send_edge->next = sender->send[send_port];
    sender->send[send_port] = send_edge;
    return true;
}


Device_node* Device_node_get_sender(Device_node* vertex,
                                    int rec_port,
                                    int* send_port)
{
    assert(vertex != NULL);
    assert(rec_port >= 0);
    assert(rec_port < KQT_DEVICE_PORTS_MAX);
    vertex->iter = vertex->receive[rec_port];
    if (vertex->iter == NULL)
    {
        return NULL;
    }
    if (send_port != NULL)
    {
        *send_port = vertex->iter->port;
    }
    return vertex->iter->vertex;
}


Device_node* Device_node_get_receiver(Device_node* vertex,
                                      int send_port,
                                      int* rec_port)
{
    assert(vertex != NULL);
    assert(send_port >= 0);
    assert(send_port < KQT_DEVICE_PORTS_MAX);
    vertex->iter = vertex->send[send_port];
    if (vertex->iter == NULL)
    {
        return NULL;
    }
    if (rec_port != NULL)
    {
        *rec_port = vertex->iter->port;
    }
    return vertex->iter->vertex;
}


Device_node* Device_node_get_next(Device_node* vertex, int* port)
{
    assert(vertex != NULL);
    if (vertex->iter == NULL)
    {
        return NULL;
    }
    vertex->iter = vertex->iter->next;
    if (vertex->iter != NULL && port != NULL)
    {
        *port = vertex->iter->port;
    }
    return vertex->iter->vertex;
}


bool Device_node_cycle_in_path(Device_node* vertex)
{
    assert(vertex != NULL);
    if (Device_node_get_state(vertex) == DEVICE_NODE_STATE_VISITED)
    {
        return false;
    }
    if (Device_node_get_state(vertex) == DEVICE_NODE_STATE_REACHED)
    {
        return true;
    }
    Device_node_set_state(vertex, DEVICE_NODE_STATE_REACHED);
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* edge = vertex->receive[i];
        while (edge != NULL)
        {
            if (Device_node_cycle_in_path(edge->vertex))
            {
                return true;
            }
            edge = edge->next;
        }
    }
    Device_node_set_state(vertex, DEVICE_NODE_STATE_VISITED);
    return false;
}


void del_Device_node(Device_node* vertex)
{
    assert(vertex != NULL);
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* cur = vertex->receive[i];
        Connection* next = NULL;
        while (cur != NULL)
        {
            next = cur->next;
            xfree(cur);
            cur = next;
        }
        cur = vertex->send[i];
        next = NULL;
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


