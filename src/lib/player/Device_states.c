

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Device_states.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <init/Connections.h>
#include <init/devices/Audio_unit.h>
#include <player/devices/Device_state.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Device_states
{
    AAtree* states;
};


Device_states* new_Device_states(void)
{
    Device_states* states = memory_alloc_item(Device_states);
    if (states == NULL)
        return NULL;

    states->states = NULL;

    states->states = new_AAtree(
            (AAtree_item_cmp*)Device_state_cmp, (AAtree_item_destroy*)del_Device_state);
    if (states->states == NULL)
    {
        del_Device_states(states);
        return NULL;
    }

    return states;
}


bool Device_states_add_state(Device_states* states, Device_state* state)
{
    rassert(states != NULL);
    rassert(state != NULL);
    rassert(!AAtree_contains(states->states, state));

    if (!AAtree_ins(states->states, state))
        return false;

    Device_state_reset(state);

    return true;
}


Device_state* Device_states_get_state(const Device_states* states, uint32_t id)
{
    rassert(states != NULL);
    rassert(id > 0);

    const Device_state* key = DEVICE_STATE_KEY(id);
    rassert(AAtree_contains(states->states, key));

    return AAtree_get_exact(states->states, key);
}


void Device_states_remove_state(Device_states* states, uint32_t id)
{
    rassert(states != NULL);
    rassert(id > 0);

    const Device_state* key = DEVICE_STATE_KEY(id);
    del_Device_state(AAtree_remove(states->states, key));

    return;
}


bool Device_states_set_audio_rate(Device_states* states, int32_t rate)
{
    rassert(states != NULL);
    rassert(rate > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_set_audio_rate(ds, rate))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


bool Device_states_set_audio_buffer_size(Device_states* states, int32_t size)
{
    rassert(states != NULL);
    rassert(size >= 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_set_audio_buffer_size(ds, size))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


/*
bool Device_states_allocate_space(Device_states* states, char* key)
{
    rassert(states != NULL);
    rassert(key != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_allocate_space(ds, key))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}
// */


void Device_states_clear_audio_buffers(
        Device_states* states, int32_t start, int32_t stop)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_clear_audio_buffers(ds, start, stop);

        ds = AAiter_get_next(iter);
    }

    return;
}


void Device_states_set_tempo(Device_states* states, double tempo)
{
    rassert(states != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_set_tempo(ds, tempo);
        ds = AAiter_get_next(iter);
    }

    return;
}


static bool init_buffers(Device_states* dstates, const Device_node* node)
{
    rassert(dstates != NULL);
    rassert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return true;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));
    rassert(Device_state_get_node_state(node_dstate) != DEVICE_NODE_STATE_REACHED);

    if (Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED)
        return true;

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);
        while (edge != NULL)
        {
            rassert(edge->node != NULL);
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
            if (!init_buffers(dstates, edge->node))
                return false;

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return true;
}


static bool init_effect_buffers(Device_states* dstates, const Device_node* node)
{
    rassert(dstates != NULL);
    rassert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return true;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_state_get_node_state(node_dstate) != DEVICE_NODE_STATE_REACHED);
        return true;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_AU)
    {
        const Audio_unit* au = Device_node_get_au_mut(node);
        if (au == NULL)
        {
            Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
            return true;
        }

        const Connections* au_conns = Audio_unit_get_connections(au);
        if (au_conns != NULL)
        {
            if (!Device_states_prepare(dstates, au_conns))
                return false;
        }
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);
        while (edge != NULL)
        {
            if (Device_node_get_device(edge->node) == NULL)
            {
                edge = edge->next;
                continue;
            }

            if (!init_effect_buffers(dstates, edge->node))
                return false;

            edge = edge->next;
        }
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return true;
}


static bool Device_states_init_buffers(Device_states* dstates, const Connections* conns)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    Device_states_reset_node_states(dstates);
    if (!init_buffers(dstates, master))
        return false;

    Device_states_reset_node_states(dstates);
    return init_effect_buffers(dstates, master);
}


bool Device_states_prepare(Device_states* dstates, const Connections* conns)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);

    return Device_states_init_buffers(dstates, conns);
}


static void process_mixed_signals(
        Device_states* dstates,
        const Device_node* node,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(dstates != NULL);
    rassert(node != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Device* node_device = Device_node_get_device(node);
    if ((node_device == NULL) || !Device_is_existent(node_device))
        return;

    Device_state* node_dstate =
        Device_states_get_state(dstates, Device_get_id(node_device));

    //fprintf(stderr, "Entering node %p %s\n", (void*)node, node->name);
    if (Device_state_get_node_state(node_dstate) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_state_get_node_state(node_dstate) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_REACHED);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

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

            Device_state* send_state =
                Device_states_get_state(dstates, Device_get_id(send_device));
            if (send_state == NULL)
            {
                edge = edge->next;
                continue;
            }

            process_mixed_signals(
                    dstates, edge->node, wbs, buf_start, buf_stop, audio_rate, tempo);

            Work_buffer* send = Device_state_get_audio_buffer(
                    send_state, DEVICE_PORT_TYPE_SEND, edge->port);
            Work_buffer* receive = Device_state_get_audio_buffer(
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
                    Work_buffer_get_contents(send)[0]);
            // */
            Work_buffer_mix(receive, send, buf_start, buf_stop);

            edge = edge->next;
        }
    }

    //fprintf(stderr, "Rendering mixed on %p %s\n", (void*)node, node->name);
    Device_state_render_mixed(node_dstate, wbs, buf_start, buf_stop, tempo);

    Device_state_set_node_state(node_dstate, DEVICE_NODE_STATE_VISITED);
    return;
}


void Device_states_process_mixed_signals(
        Device_states* dstates,
        bool hack_reset,
        const Connections* conns,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(audio_rate > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    if (buf_start >= buf_stop)
        return;

#if 0
    static bool called = false;
    if (!called)
    {
        Connections_print(graph, stderr);
    }
    called = true;
//    fprintf(stderr, "Mix process:\n");
#endif

    if (hack_reset)
        Device_states_reset_node_states(dstates);

    process_mixed_signals(
            dstates, master, wbs, buf_start, buf_stop, audio_rate, tempo);

    return;
}


void Device_states_reset(Device_states* states)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_reset(ds);
        ds = AAiter_get_next(iter);
    }

    return;
}


void Device_states_reset_node_states(Device_states* states)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_set_node_state(ds, DEVICE_NODE_STATE_NEW);
        ds = AAiter_get_next(iter);
    }

    return;
}


void del_Device_states(Device_states* states)
{
    if (states == NULL)
        return;

    del_AAtree(states->states);
    memory_free(states);
    return;
}


