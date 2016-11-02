

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/Device_node.h>

#include <debug/assert.h>
#include <init/Connections.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Voice_state.h>
#include <player/Voice_group.h>
#include <player/Work_buffer.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Device_node
{
    char name[KQT_DEVICE_NODE_NAME_MAX];

    Device_node_state cycle_test_state;

    // These fields are required for adaptation to changes
    Au_table* au_table;
    const Device* master; ///< The global or Audio unit master

    Device_node_type type;
    int index;
    //Device* device;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


Processor* Device_node_get_processor_mut(const Device_node* node);


Device_node* new_Device_node(const char* name, Au_table* au_table, const Device* master)
{
    rassert(name != NULL);
    rassert(au_table != NULL);
    rassert(master != NULL);

    Device_node* node = memory_alloc_item(Device_node);
    if (node == NULL)
        return NULL;

    strncpy(node->name, name, KQT_DEVICE_NODE_NAME_MAX - 1);

    if (string_eq(node->name, ""))
    {
        node->type = DEVICE_NODE_TYPE_MASTER;
        node->index = -1;
    }
    else if (string_has_prefix(node->name, "au_"))
    {
        node->type = DEVICE_NODE_TYPE_AU;
        node->index = string_extract_index(node->name, "au_", 2, NULL);
        rassert(node->index >= 0);
        rassert(node->index < KQT_AUDIO_UNITS_MAX);
    }
    else if (string_has_prefix(node->name, "proc_"))
    {
        node->type = DEVICE_NODE_TYPE_PROCESSOR;
        node->index = string_extract_index(node->name, "proc_", 2, NULL);
        rassert(node->index >= 0);
        rassert(node->index < KQT_PROCESSORS_MAX);
    }
    else if (string_eq(node->name, "Iin"))
    {
        node->type = DEVICE_NODE_TYPE_MASTER;
        node->index = -1;
    }
    else
    {
        rassert(false);
    }

    node->cycle_test_state = DEVICE_NODE_STATE_NEW;
    node->au_table = au_table;
    node->master = master;
    //node->device = NULL;
    node->name[KQT_DEVICE_NODE_NAME_MAX - 1] = '\0';
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
    rassert(n1 != NULL);
    rassert(n2 != NULL);

    return strcmp(n1->name, n2->name);
}


bool Device_node_check_connections(
        const Device_node* node, char err[DEVICE_CONNECTION_ERROR_LENGTH_MAX])
{
    rassert(node != NULL);
    rassert(err != NULL);

    static const char* master_name = "master";

    const Device* recv_device = Device_node_get_device(node);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        bool has_connections = false;

        Connection* conn = node->receive[port];
        while (conn != NULL)
        {
            has_connections = true;

            const char* send_dev_name = conn->node->name;
            if (send_dev_name[0] == '\0')
                send_dev_name = master_name;

            // Check existence of sending device
            const Device* send_device = Device_node_get_device(conn->node);
            if ((send_device == NULL) || !Device_is_existent(send_device))
            {
                snprintf(
                        err, DEVICE_CONNECTION_ERROR_LENGTH_MAX,
                        "Device %s does not exist",
                        send_dev_name);
                return false;
            }

            // Check existence of send port
            if (!Device_get_port_existence(
                        send_device, DEVICE_PORT_TYPE_SEND, conn->port))
            {
                snprintf(
                        err, DEVICE_CONNECTION_ERROR_LENGTH_MAX,
                        "Device %s does not have send port %d",
                        send_dev_name,
                        conn->port);
                return false;
            }

            conn = conn->next;
        }

        if (has_connections)
        {
            const char* recv_dev_name = node->name;
            if (recv_dev_name[0] == '\0')
                recv_dev_name = master_name;

            if ((recv_device == NULL) || !Device_is_existent(recv_device))
            {
                snprintf(
                        err, DEVICE_CONNECTION_ERROR_LENGTH_MAX,
                        "Device %s does not exist",
                        recv_dev_name);
                return false;
            }

            if (!Device_get_port_existence(
                        recv_device, DEVICE_PORT_TYPE_RECEIVE, port))
            {
                snprintf(
                        err, DEVICE_CONNECTION_ERROR_LENGTH_MAX,
                        "Device %s does not have receive port %d",
                        recv_dev_name,
                        port);
                return false;
            }
        }
    }

    return true;
}


void Device_node_reset_subgraph(const Device_node* node, Device_states* dstates)
{
    rassert(node != NULL);
    rassert(dstates != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) < DEVICE_NODE_STATE_VISITED)
    {
        rassert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_NEW);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = node->receive[port];

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            Device_node_reset_subgraph(edge->node, dstates);

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_NEW);

    return;
}


int32_t Device_node_process_voice_group(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(node != NULL);
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);
    rassert(tempo > 0);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return buf_start;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return buf_start;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    Proc_state* recv_state = NULL;
    if (node->type == DEVICE_NODE_TYPE_PROCESSOR)
    {
        recv_state = (Proc_state*)node_dstate;

        // Clear the voice buffers for new contents
        Proc_state_clear_voice_buffers(recv_state);

        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            // Stop recursing if we don't have an active Voice
            const uint32_t proc_id = Device_get_id(node_device);
            Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);
            if ((voice == NULL) || (!voice->state->active))
            {
                Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
                return buf_start;
            }
        }
    }

    int32_t release_stop = buf_start;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = node->receive[port];

        if (edge != NULL)
            Device_state_mark_input_port_connected(node_dstate, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            const int32_t sub_release_stop = Device_node_process_voice_group(
                    edge->node,
                    vgroup,
                    dstates,
                    wbs,
                    buf_start,
                    buf_stop,
                    audio_rate,
                    tempo);

            release_stop = max(release_stop, sub_release_stop);

            if ((node->type == DEVICE_NODE_TYPE_PROCESSOR) &&
                    (edge->node->type == DEVICE_NODE_TYPE_PROCESSOR))
            {
                // Mix voice audio buffers
                Proc_state* send_state = (Proc_state*)Device_states_get_state(
                        dstates, Device_get_id(send_device));
                const Work_buffer* send_buf = Proc_state_get_voice_buffer(
                        send_state, DEVICE_PORT_TYPE_SEND, edge->port);

                Work_buffer* recv_buf = Proc_state_get_voice_buffer_mut(
                        recv_state, DEVICE_PORT_TYPE_RECEIVE, port);

                if ((send_buf != NULL) && (recv_buf != NULL))
                    Work_buffer_mix(recv_buf, send_buf, buf_start, buf_stop);
            }

            edge = edge->next;
        }
    }

    if (node->type == DEVICE_NODE_TYPE_PROCESSOR)
    {
        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            // Find the Voice that belongs to the current Processor
            const uint32_t proc_id = Device_get_id(node_device);
            Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);

            if (voice != NULL)
            {
                const int32_t voice_release_stop = Voice_render(
                        voice, dstates, wbs, buf_start, buf_stop, tempo);
                release_stop = max(release_stop, voice_release_stop);
            }
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);

    return release_stop;
}


void Device_node_mix_voice_signals(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(node != NULL);
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    if (node->type == DEVICE_NODE_TYPE_PROCESSOR)
    {
        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            // Mix Voice signals if we have any
            const uint32_t proc_id = Device_get_id(node_device);
            Proc_state* pstate = (Proc_state*)Device_states_get_state(dstates, proc_id);
            Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);
            if (voice != NULL)
                Voice_state_mix_signals(voice->state, pstate, buf_start, buf_stop);

            // Stop recursing as we don't depend on any mixed signals
            Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
            return;
        }
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = node->receive[port];

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            Device_node_mix_voice_signals(
                    edge->node, vgroup, dstates, buf_start, buf_stop);

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);

    return;
}


const char* Device_node_get_name(const Device_node* node)
{
    rassert(node != NULL);
    return node->name;
}


Processor* Device_node_get_processor_mut(const Device_node* node)
{
    rassert(node != NULL);
    rassert(node->type == DEVICE_NODE_TYPE_PROCESSOR);

    Proc_table* procs = Audio_unit_get_procs((const Audio_unit*)node->master);
    return Proc_table_get_proc_mut(procs, node->index);
}


const Device* Device_node_get_device(const Device_node* node)
{
    rassert(node != NULL);

    if (node->type == DEVICE_NODE_TYPE_MASTER)
        return node->master;
    else if (node->type == DEVICE_NODE_TYPE_AU)
        return (const Device*)Au_table_get(node->au_table, node->index);
    else if (node->type == DEVICE_NODE_TYPE_PROCESSOR)
        return (const Device*)Audio_unit_get_proc(
                (const Audio_unit*)node->master,
                node->index);

    rassert(false);
    return NULL;
}


const Connection* Device_node_get_received(const Device_node* node, int port)
{
    rassert(node != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return node->receive[port];
}


Device_node_type Device_node_get_type(const Device_node* node)
{
    rassert(node != NULL);
    return node->type;
}


Audio_unit* Device_node_get_au_mut(const Device_node* node)
{
    rassert(node != NULL);
    rassert(Device_node_get_type(node) == DEVICE_NODE_TYPE_AU);

    return Au_table_get(node->au_table, node->index);
}


bool Device_node_connect(
        Device_node* receiver, int rec_port, Device_node* sender, int send_port)
{
    rassert(receiver != NULL);
    rassert(rec_port >= 0);
    rassert(rec_port < KQT_DEVICE_PORTS_MAX);
    rassert(sender != NULL);
    rassert(send_port >= 0);
    rassert(send_port < KQT_DEVICE_PORTS_MAX);

    Connection* receive_edge = memory_alloc_item(Connection);
    if (receive_edge == NULL)
        return false;

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


void Device_node_reset_cycle_test_state(Device_node* node)
{
    rassert(node != NULL);
    node->cycle_test_state = DEVICE_NODE_STATE_NEW;
    return;
}


bool Device_node_cycle_in_path(Device_node* node)
{
    rassert(node != NULL);

    if (node->cycle_test_state == DEVICE_NODE_STATE_VISITED)
        return false;

    if (node->cycle_test_state == DEVICE_NODE_STATE_REACHED)
        return true;

    node->cycle_test_state = DEVICE_NODE_STATE_REACHED;
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        Connection* edge = node->receive[i];
        while (edge != NULL)
        {
            if (Device_node_cycle_in_path(edge->node))
                return true;

            edge = edge->next;
        }
    }

    node->cycle_test_state = DEVICE_NODE_STATE_VISITED;
    return false;
}


void Device_node_print(const Device_node* node, FILE* out)
{
    rassert(node != NULL);
    rassert(out != NULL);

    static const char* states[] =
    {
        [DEVICE_NODE_STATE_NEW] = "new",
        [DEVICE_NODE_STATE_REACHED] = "reached",
        [DEVICE_NODE_STATE_VISITED] = "visited",
    };

    fprintf(out,
            "\nDevice node: %s, Device: %p, (state: %s)\n",
            node->name[0] == '\0' ? "[Master]" : node->name,
            (const void*)Device_node_get_device(node),
            states[node->cycle_test_state]);

    if (Device_node_get_device(node) != NULL)
        Device_print(Device_node_get_device(node), out);

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

            fprintf(out,
                    "    %s (device %p), send port %02x\n",
                    edge->node->name,
                    (const void*)Device_node_get_device(edge->node),
                    edge->port);

            edge = edge->next;
        }
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            rassert(edge->node->send[edge->port] != NULL);
            if (node == edge->node->send[edge->port]->node)
                Device_node_print(edge->node, out);

            edge = edge->next;
        }
    }

    return;
}


void del_Device_node(Device_node* node)
{
    if (node == NULL)
        return;

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


