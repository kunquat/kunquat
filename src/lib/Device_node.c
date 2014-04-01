

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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
#include <devices/Generator.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


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
    Effect_table* effects;
    DSP_table* dsps;
    Device* master; ///< The global, Instrument or Effect master

    Device_type type;
    int index;
    //Device* device;
    Device_node_state state;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


static Device_node* Device_node_get_ins_dual(Device_node* node);


Device_node* new_Device_node(
        const char* name,
        Ins_table* insts,
        Effect_table* effects,
        DSP_table* dsps,
        Device* master)
{
    assert(name != NULL);
    assert(insts != NULL);
    assert(effects != NULL);
    assert(master != NULL);
    Device_node* node = memory_alloc_item(Device_node);
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
        node->index = string_extract_index(node->name, "ins_", 2, NULL);
        assert(node->index >= 0);
        assert(node->index < KQT_INSTRUMENTS_MAX);
    }
    else if (string_has_prefix(node->name, "gen_"))
    {
        node->type = DEVICE_TYPE_GENERATOR;
        node->index = string_extract_index(node->name, "gen_", 2, NULL);
        assert(node->index >= 0);
        assert(node->index < KQT_GENERATORS_MAX);
    }
    else if (string_has_prefix(node->name, "eff_"))
    {
        node->type = DEVICE_TYPE_EFFECT;
        node->index = string_extract_index(node->name, "eff_", 2, NULL);
        assert(node->index >= 0);
        // TODO: upper bound
    }
    else if (string_eq(node->name, "Iin"))
    {
        assert(dsps != NULL);
        node->type = DEVICE_TYPE_MASTER;
        node->index = -1;
    }
    else if (string_has_prefix(node->name, "dsp_"))
    {
        node->type = DEVICE_TYPE_DSP;
        node->index = string_extract_index(node->name, "dsp_", 2, NULL);
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
    node->effects = effects;
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


bool Device_node_init_buffers_simple(Device_node* node, Device_states* states)
{
    assert(node != NULL);
    assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
    assert(states != NULL);

    if (Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED)
        return true;

    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);

    Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
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
            Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL || !Device_has_complete_type(send_device))
            {
                edge = edge->next;
                continue;
            }

            // Add receive buffer
            Device_state* receive_state = Device_states_get_state(
                    states,
                    Device_get_id(node_device));
            if (!Device_state_add_audio_buffer(
                        receive_state,
                        DEVICE_PORT_TYPE_RECEIVE,
                        port))
                return false;

            // Add send buffer
            Device_state* send_state = Device_states_get_state(
                    states,
                    Device_get_id(send_device));
            if (send_state != NULL &&
                    !Device_state_add_audio_buffer(
                        send_state,
                        DEVICE_PORT_TYPE_SEND,
                        edge->port))
                return false;

            // Recurse to the sender
            if (!Device_node_init_buffers_simple(edge->node, states))
                return false;

            edge = edge->next;
        }
    }

    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return true;
}


bool Device_node_init_effect_buffers(Device_node* node, Device_states* states)
{
    assert(node != NULL);
    assert(states != NULL);

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
    else if (node->type == DEVICE_TYPE_EFFECT)
    {
        Effect* eff = Effect_table_get(node->effects, node->index);
        if (eff == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return true;
        }

        if (!Effect_prepare_connections(eff, states))
            return false;
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

            if (!Device_node_init_effect_buffers(edge->node, states))
                return false;

            edge = edge->next;
        }
    }

    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return true;
}


void Device_node_clear_buffers(
        Device_node* node,
        Device_states* states,
        uint32_t start,
        uint32_t until)
{
    assert(node != NULL);
    assert(states != NULL);

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
    const Device* device = Device_node_get_device(node);
    Device_state* ds = Device_states_get_state(states, Device_get_id(device));

    if (ds != NULL)
        Device_state_clear_audio_buffers(ds, start, until);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_clear_buffers(edge->node, states, start, until);
            edge = edge->next;
        }
    }

    Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
    return;
}


void Device_node_mix(
        Device_node* node,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(node != NULL);
    assert(states != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    //fprintf(stderr, "Entering node %p %s\n", (void*)node, node->name);
    if (Device_node_get_state(node) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_node_get_state(node) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_node_set_state(node, DEVICE_NODE_STATE_REACHED);
    Device* node_device = Device_node_get_device(node);
    Device_state* ds = Device_states_get_state(
            states,
            Device_get_id(node_device));
    if (node_device == NULL ||
            !Device_is_existent(node_device) ||
            ds == NULL)
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
        Device_state* ins_state = Device_states_get_state(
                states,
                Device_get_id((Device*)ins));
        if (ins_graph == NULL ||
                (ins_node = Connections_get_master(ins_graph)) == NULL ||
                ins_state == NULL)
        {
            Device_node_set_state(node, DEVICE_NODE_STATE_VISITED);
            return;
        }

        // Mix audio inside the instrument
        Device_node_mix(ins_node, states, start, until, freq, tempo);

        // Copy audio to instrument front end
        for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
        {
            Audio_buffer* receive = Device_state_get_audio_buffer(
                    ins_state,
                    DEVICE_PORT_TYPE_RECEIVE,
                    port);
            Audio_buffer* send = Device_state_get_audio_buffer(
                    ins_state,
                    DEVICE_PORT_TYPE_SEND,
                    port);

            if (receive != NULL && send != NULL)
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
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            Device_state* send_state = Device_states_get_state(
                    states,
                    Device_get_id(send_device));
            if (send_state == NULL)
            {
                edge = edge->next;
                continue;
            }

            Device_node_mix(edge->node, states, start, until, freq, tempo);

            Audio_buffer* send = Device_state_get_audio_buffer(
                    send_state,
                    DEVICE_PORT_TYPE_SEND,
                    edge->port);
            Audio_buffer* receive = Device_state_get_audio_buffer(
                    ds,
                    DEVICE_PORT_TYPE_RECEIVE,
                    port);
            if (receive == NULL || send == NULL)
            {
#if 0
                if (receive != NULL)
                {
                    fprintf(stderr, "receive %p of %p %s, but no send from %p!\n",
                            (void*)receive, (void*)node, node->name, (void*)edge->node);
                }
                else if (send != NULL)
                {
                    fprintf(stderr, "send %p, but no receive!\n", (void*)send);
                }
#endif
                edge = edge->next;
                continue;
            }

            //fprintf(stderr, "%s %.1f\n",
            //        send_device->name,
            //        Audio_buffer_get_buffer(send, 0)[0]);
            Audio_buffer_mix(receive, send, start, until);

            edge = edge->next;
        }
    }

    //fprintf(stderr, "Calling Device_process on %p %s\n", (void*)node, node->name);
    Device_process(node_device, states, start, until, freq, tempo);
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
        return node->master;
    else if (node->type == DEVICE_TYPE_INSTRUMENT)
        return (Device*)Ins_table_get(node->insts, node->index);
    else if (node->type == DEVICE_TYPE_EFFECT)
        return (Device*)Effect_table_get(node->effects, node->index);
    else if (node->type == DEVICE_TYPE_GENERATOR)
        return (Device*)Instrument_get_gen(
                (Instrument*)node->master,
                node->index);
    else if (node->type == DEVICE_TYPE_DSP)
        return (Device*)DSP_table_get_dsp(node->dsps, node->index);

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
    Connection* receive_edge = memory_alloc_item(Connection);
    if (receive_edge == NULL)
    {
        return false;
    }
    Connection* send_edge = memory_alloc_item(Connection);
    if (send_edge == NULL)
    {
        memory_free(receive_edge);
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
            memory_free(cur);
            cur = next;
        }
        cur = node->send[i];
        next = NULL;
        while (cur != NULL)
        {
            next = cur->next;
            memory_free(cur);
            cur = next;
        }
    }
    memory_free(node);
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


