

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Device_node.h>

#include <Connections.h>
#include <debug/assert.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Voice_state.h>
#include <player/Voice_group.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum
{
    DEVICE_TYPE_MASTER    = 1,
    DEVICE_TYPE_PROCESSOR = 2,
    DEVICE_TYPE_AU        = 8,
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

    Device_node_state cycle_test_state;

    // These fields are required for adaptation to changes
    Au_table* au_table;
    const Device* master; ///< The global or Audio unit master

    Device_type type;
    int index;
    //Device* device;
    Connection* iter;
    Connection* receive[KQT_DEVICE_PORTS_MAX];
    Connection* send[KQT_DEVICE_PORTS_MAX];
};


Processor* Device_node_get_processor_mut(const Device_node* node);


Device_node* new_Device_node(const char* name, Au_table* au_table, const Device* master)
{
    assert(name != NULL);
    assert(au_table != NULL);
    assert(master != NULL);

    Device_node* node = memory_alloc_item(Device_node);
    if (node == NULL)
        return NULL;

    strncpy(node->name, name, KQT_DEVICE_NODE_NAME_MAX - 1);

    if (string_eq(node->name, ""))
    {
        node->type = DEVICE_TYPE_MASTER;
        node->index = -1;
    }
    else if (string_has_prefix(node->name, "au_"))
    {
        node->type = DEVICE_TYPE_AU;
        node->index = string_extract_index(node->name, "au_", 2, NULL);
        assert(node->index >= 0);
        assert(node->index < KQT_AUDIO_UNITS_MAX);
    }
    else if (string_has_prefix(node->name, "proc_"))
    {
        node->type = DEVICE_TYPE_PROCESSOR;
        node->index = string_extract_index(node->name, "proc_", 2, NULL);
        assert(node->index >= 0);
        assert(node->index < KQT_PROCESSORS_MAX);
    }
    else if (string_eq(node->name, "Iin"))
    {
        node->type = DEVICE_TYPE_MASTER;
        node->index = -1;
    }
    else
    {
        assert(false);
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
    assert(n1 != NULL);
    assert(n2 != NULL);

    return strcmp(n1->name, n2->name);
}


void Device_node_clear_processor_voice_cut_settings(Device_node* node)
{
    assert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    if (node->type == DEVICE_TYPE_PROCESSOR)
    {
        Processor* proc = Device_node_get_processor_mut(node);
        Processor_set_voice_feature(proc, 0, VOICE_FEATURE_CUT, false);
    }

    return;
}


void Device_node_init_processor_voice_cut_settings(Device_node* node)
{
    assert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    bool is_mixed_device = true;
    if (node->type == DEVICE_TYPE_PROCESSOR)
        is_mixed_device = Device_get_mixed_signals(node_device);

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

            if (is_mixed_device && (edge->node->type == DEVICE_TYPE_PROCESSOR))
            {
                Processor* proc = Device_node_get_processor_mut(edge->node);
                if (proc != NULL)
                    Processor_set_voice_feature(proc, 0, VOICE_FEATURE_CUT, true);
            }

            edge = edge->next;
        }
    }

    return;
}


bool Device_node_check_connections(
        const Device_node* node, char err[DEVICE_CONNECTION_ERROR_LENGTH_MAX])
{
    assert(node != NULL);
    assert(err != NULL);

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

            // Check that Processors connected to mixed signal devices
            // have the voice cut feature enabled
            if (((node->type == DEVICE_TYPE_MASTER) ||
                        Device_get_mixed_signals(recv_device)) &&
                    (conn->node->type == DEVICE_TYPE_PROCESSOR))
            {
                const Processor* proc = (const Processor*)send_device;
                if (Processor_get_voice_signals(proc) &&
                        !Processor_is_voice_feature_enabled(
                            proc, conn->port, VOICE_FEATURE_CUT))
                {
                    snprintf(
                            err, DEVICE_CONNECTION_ERROR_LENGTH_MAX,
                            "Device %s port %d is connected to a mixed signal device"
                            " but has voice cutting feature disabled",
                            send_dev_name, conn->port);
                    return false;
                }
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


bool Device_node_init_buffers_simple(const Device_node* node, Device_states* dstates)
{
    assert(node != NULL);
    assert(dstates != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return true;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));
    assert(Device_state_get_node_state(node_dstate) != DEVICE_NODE_STATE_REACHED);

    if (Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED)
        return true;

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            assert(edge->node != NULL);
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL ||
                    !Device_has_complete_type(send_device) ||
                    !Device_get_port_existence(
                        node_device, DEVICE_PORT_TYPE_RECEIVE, port) ||
                    !Device_get_port_existence(
                        send_device, DEVICE_PORT_TYPE_SEND, edge->port))
            {
                edge = edge->next;
                continue;
            }
            /*
            if (!Device_get_port_existence(
                    node_device, DEVICE_PORT_TYPE_RECEIVE, port))
                fprintf(stderr, "Warning: connecting to non-existent port %d of device %s\n",
                        port, node->name);
            if (!Device_get_port_existence(
                    send_device, DEVICE_PORT_TYPE_SEND, edge->port))
                fprintf(stderr, "Warning: connecting from non-existent port %d of device %s\n",
                        edge->port, edge->node->name);
            // */

            // Add receive buffer
            Device_state* receive_state =
                Device_states_get_state(dstates, Device_get_id(node_device));
            if (!Device_state_add_audio_buffer(
                        receive_state, DEVICE_PORT_TYPE_RECEIVE, port))
                return false;

            // Add send buffer
            Device_state* send_state =
                Device_states_get_state(dstates, Device_get_id(send_device));
            if (send_state != NULL &&
                    !Device_state_add_audio_buffer(
                        send_state, DEVICE_PORT_TYPE_SEND, edge->port))
                return false;

            // Recurse to the sender
            if (!Device_node_init_buffers_simple(edge->node, dstates))
                return false;

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return true;
}


bool Device_node_init_effect_buffers(const Device_node* node, Device_states* dstates)
{
    assert(node != NULL);
    assert(dstates != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return true;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_state_get_node_state(node_dstate) != DEVICE_NODE_STATE_REACHED);
        return true;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    if (node->type == DEVICE_TYPE_AU)
    {
        Audio_unit* au = Au_table_get(node->au_table, node->index);
        if (au == NULL)
        {
            Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
            return true;
        }

        if (!Audio_unit_prepare_connections(au, dstates))
            return false;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }

            if (!Device_node_init_effect_buffers(edge->node, dstates))
                return false;

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return true;
}


void Device_node_clear_buffers(
        const Device_node* node,
        Device_states* dstates,
        uint32_t start,
        uint32_t until)
{
    assert(node != NULL);
    assert(dstates != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    //fprintf(stderr, "Clearing buffers of %p\n", (void*)Device_node_get_device(node));

    if (node->type == DEVICE_TYPE_AU)
    {
        // Clear the audio unit buffers
        const Audio_unit* au = (const Audio_unit*)node_device;
        const Connections* au_conns = Audio_unit_get_connections(au);
        if (au_conns != NULL)
            Connections_clear_buffers(au_conns, dstates, start, until);
    }

    Device_state_clear_audio_buffers(node_dstate, start, until);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Connection* edge = node->receive[port];
        while (edge != NULL)
        {
            Device_node_clear_buffers(edge->node, dstates, start, until);
            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return;
}


void Device_node_process_voice_group(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(node != NULL);
    assert(vgroup != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);
    assert(tempo > 0);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    Proc_state* recv_state = NULL;
    if (node->type == DEVICE_TYPE_PROCESSOR)
    {
        recv_state = (Proc_state*)Device_states_get_state(
                dstates, Device_get_id(node_device));

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
                return;
            }
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

            Device_node_process_voice_group(
                    edge->node,
                    vgroup,
                    dstates,
                    wbs,
                    buf_start,
                    buf_stop,
                    audio_rate,
                    tempo);

            if ((node->type == DEVICE_TYPE_PROCESSOR) &&
                    (edge->node->type == DEVICE_TYPE_PROCESSOR))
            {
                // Mix voice audio buffers
                Proc_state* send_state = (Proc_state*)Device_states_get_state(
                        dstates, Device_get_id(send_device));
                const Audio_buffer* send_buf = Proc_state_get_voice_buffer(
                        send_state, DEVICE_PORT_TYPE_SEND, edge->port);

                Audio_buffer* recv_buf = Proc_state_get_voice_buffer_mut(
                        recv_state, DEVICE_PORT_TYPE_RECEIVE, port);

                if ((send_buf != NULL) && (recv_buf != NULL))
                    Audio_buffer_mix(recv_buf, send_buf, buf_start, buf_stop);
            }

            edge = edge->next;
        }
    }

    if (node->type == DEVICE_TYPE_PROCESSOR)
    {
        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            // Find the Voice that belongs to the current Processor
            const uint32_t proc_id = Device_get_id(node_device);
            Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);

            if (voice != NULL)
                Voice_mix(voice, dstates, wbs, buf_stop, buf_start, audio_rate, tempo);
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return;
}


void Device_node_mix(
        const Device_node* node,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(node != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    const Device* node_device = Device_node_get_device(node);
    if ((node_device == NULL) || !Device_is_existent(node_device))
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    //fprintf(stderr, "Entering node %p %s\n", (void*)node, node->name);
    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        assert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

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

            Device_state* send_state =
                Device_states_get_state(dstates, Device_get_id(send_device));
            if (send_state == NULL)
            {
                edge = edge->next;
                continue;
            }

            Device_node_mix(edge->node, dstates, wbs, start, until, freq, tempo);

            Audio_buffer* send = Device_state_get_audio_buffer(
                    send_state, DEVICE_PORT_TYPE_SEND, edge->port);
            Audio_buffer* receive = Device_state_get_audio_buffer(
                    node_dstate, DEVICE_PORT_TYPE_RECEIVE, port);
            if (receive == NULL || send == NULL)
            {
                /*
                if (receive != NULL)
                {
                    fprintf(stderr, "receive %p of %p %s, but no send from %p!\n",
                            (void*)receive, (void*)node, node->name, (void*)edge->node);
                }
                else if (send != NULL)
                {
                    fprintf(stderr, "send %p, but no receive!\n", (void*)send);
                }
                // */
                edge = edge->next;
                continue;
            }

            /*
            fprintf(stderr, "%s %d %.1f\n",
                    edge->node->name,
                    (int)Device_get_id((const Device*)send_device),
                    Audio_buffer_get_buffer(send, 0)[0]);
            // */
            Audio_buffer_mix(receive, send, start, until);

            edge = edge->next;
        }
    }

    //fprintf(stderr, "Rendering mixed on %p %s\n", (void*)node, node->name);
    Device_state_render_mixed(node_dstate, wbs, start, until, tempo);

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return;
}


const char* Device_node_get_name(const Device_node* node)
{
    assert(node != NULL);
    return node->name;
}


Processor* Device_node_get_processor_mut(const Device_node* node)
{
    assert(node != NULL);
    assert(node->type == DEVICE_TYPE_PROCESSOR);

    Proc_table* procs = Audio_unit_get_procs((const Audio_unit*)node->master);
    return Proc_table_get_proc_mut(procs, node->index);
}


const Device* Device_node_get_device(const Device_node* node)
{
    assert(node != NULL);

    if (node->type == DEVICE_TYPE_MASTER)
        return node->master;
    else if (node->type == DEVICE_TYPE_AU)
        return (const Device*)Au_table_get(node->au_table, node->index);
    else if (node->type == DEVICE_TYPE_PROCESSOR)
        return (const Device*)Audio_unit_get_proc(
                (const Audio_unit*)node->master,
                node->index);

    assert(false);
    return NULL;
}


bool Device_node_connect(
        Device_node* receiver, int rec_port, Device_node* sender, int send_port)
{
    assert(receiver != NULL);
    assert(rec_port >= 0);
    assert(rec_port < KQT_DEVICE_PORTS_MAX);
    assert(sender != NULL);
    assert(send_port >= 0);
    assert(send_port < KQT_DEVICE_PORTS_MAX);

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
    assert(node != NULL);
    node->cycle_test_state = DEVICE_NODE_STATE_NEW;
    return;
}


bool Device_node_cycle_in_path(Device_node* node)
{
    assert(node != NULL);

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
    assert(node != NULL);
    assert(out != NULL);

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
            assert(edge->node->send[edge->port] != NULL);
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


