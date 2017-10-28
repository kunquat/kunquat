

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice_group.h>

#include <debug/assert.h>
#include <init/Connections.h>
#include <init/Device_node.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <player/devices/Device_thread_state.h>
#include <player/Voice.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


Voice_group* Voice_group_init(
        Voice_group* vg, Voice** voices, int offset, int vp_size)
{
    rassert(vg != NULL);
    rassert(voices != NULL);
    rassert(offset >= 0);
    rassert(offset < vp_size);
    rassert(vp_size > 0);

    vg->voices = voices + offset;

    const uint64_t group_id = Voice_get_group_id(voices[offset]);
    if (group_id == 0)
    {
        vg->size = 0;
        return vg;
    }

    vg->size = 1;

    for (int i = offset + 1; i < vp_size; ++i)
    {
        rassert(voices[i] != NULL);
        if (Voice_get_group_id(voices[i]) != group_id)
            break;
        ++vg->size;
    }

    for (int i = 0; i < vg->size; ++i)
        vg->voices[i]->updated = false;

    return vg;
}


int Voice_group_get_size(const Voice_group* vg)
{
    rassert(vg != NULL);
    return vg->size;
}


int Voice_group_get_active_count(const Voice_group* vg)
{
    rassert(vg != NULL);

    int count = 0;
    for (int i = 0; i < vg->size; ++i)
    {
        if (vg->voices[i]->state->active)
            ++count;
    }

    return count;
}


Voice* Voice_group_get_voice(Voice_group* vg, int index)
{
    rassert(vg != NULL);
    rassert(index >= 0);
    rassert(index < Voice_group_get_size(vg));

    return vg->voices[index];
}


Voice* Voice_group_get_voice_by_proc(Voice_group* vg, uint32_t proc_id)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
    {
        const Processor* proc = Voice_get_proc(vg->voices[i]);
        if ((proc != NULL) && (Device_get_id((const Device*)proc) == proc_id))
            return vg->voices[i];
    }

    return NULL;
}


static void reset_subgraph(
        Device_states* dstates, int thread_id, const Device_node* node)
{
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_thread_state* node_ts =
        Device_states_get_thread_state(dstates, thread_id, Device_get_id(node_device));

    if (Device_thread_state_get_node_state(node_ts) < DEVICE_NODE_STATE_VISITED)
    {
        rassert(Device_thread_state_get_node_state(node_ts) == DEVICE_NODE_STATE_NEW);
        return;
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_REACHED);

    const int last_port = Device_node_get_last_receive_port(node);
    for (int port = 0; port <= last_port; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            reset_subgraph(dstates, thread_id, edge->node);

            edge = edge->next;
        }
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_NEW);

    return;
}


static int32_t process_voice_group(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        int thread_id,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(node != NULL);
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);
    rassert(tempo > 0);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return buf_start;

    Device_thread_state* node_ts =
        Device_states_get_thread_state(dstates, thread_id, Device_get_id(node_device));

    if (Device_thread_state_get_node_state(node_ts) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_thread_state_get_node_state(node_ts) == DEVICE_NODE_STATE_VISITED);
        return buf_start;
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_REACHED);

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        // Clear the voice buffers for new contents
        Device_thread_state_clear_voice_buffers(node_ts, buf_start, buf_stop);

        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            if ((node_device->dimpl->get_vstate_size == NULL) ||
                    (node_device->dimpl->get_vstate_size() > 0))
            {
                // Stop recursing if we don't have an active Voice
                const uint32_t proc_id = Device_get_id(node_device);
                Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);
                if ((voice == NULL) || (!voice->state->active))
                {
                    Device_thread_state_set_node_state(
                            node_ts, DEVICE_NODE_STATE_VISITED);
                    return buf_start;
                }
            }
        }
    }

    int32_t keep_alive_stop = buf_start;

    const int last_port = Device_node_get_last_receive_port(node);
    for (int port = 0; port <= last_port; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

        if (edge != NULL)
            Device_thread_state_mark_input_port_connected(node_ts, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            const int32_t sub_keep_alive_stop = process_voice_group(
                    edge->node,
                    vgroup,
                    dstates,
                    thread_id,
                    wbs,
                    buf_start,
                    buf_stop,
                    audio_rate,
                    tempo);

            keep_alive_stop = max(keep_alive_stop, sub_keep_alive_stop);

            if ((Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR) &&
                    (Device_node_get_type(edge->node) == DEVICE_NODE_TYPE_PROCESSOR))
            {
                // Mix voice audio buffers
                const Device_thread_state* send_ts = Device_states_get_thread_state(
                        dstates, thread_id, Device_get_id(send_device));
                const Work_buffer* send_buf = Device_thread_state_get_voice_buffer(
                        send_ts, DEVICE_PORT_TYPE_SEND, edge->port);

                Work_buffer* recv_buf = Device_thread_state_get_voice_buffer(
                        node_ts, DEVICE_PORT_TYPE_RECV, port);

                if ((send_buf != NULL) && (recv_buf != NULL))
                    Work_buffer_mix(recv_buf, send_buf, buf_start, buf_stop);
            }

            edge = edge->next;
        }
    }

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            Voice* voice = NULL;
            const uint32_t proc_id = Device_get_id(node_device);
            bool call_render = true;

            if ((node_device->dimpl->get_vstate_size == NULL) ||
                    (node_device->dimpl->get_vstate_size() > 0))
            {
                // Find the Voice that belongs to the current Processor
                voice = Voice_group_get_voice_by_proc(vgroup, proc_id);
                call_render = (voice != NULL);
            }

            if (call_render)
            {
                const int32_t voice_keep_alive_stop = Voice_render(
                        voice,
                        proc_id,
                        dstates,
                        thread_id,
                        wbs,
                        buf_start,
                        buf_stop,
                        tempo);
                keep_alive_stop = max(keep_alive_stop, voice_keep_alive_stop);
            }
        }
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_VISITED);

    return keep_alive_stop;
}


int32_t Voice_group_render(
        Voice_group* vgroup,
        Device_states* dstates,
        int thread_id,
        const Connections* conns,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(conns != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);
    rassert(tempo > 0);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    if (buf_start >= buf_stop)
        return buf_start;

    reset_subgraph(dstates, thread_id, master);
    //Device_states_reset_node_states(dstates);
    return process_voice_group(
            master,
            vgroup,
            dstates,
            thread_id,
            wbs,
            buf_start,
            buf_stop,
            audio_rate,
            tempo);
}


int Voice_group_get_ch_num(const Voice_group* vg)
{
    rassert(vg != NULL);
    rassert(vg->size > 0);

    // Try to return a valid channel number
    // (some voices in the group might have been reset)
    for (int i = 0; i < vg->size; ++i)
    {
        const int ch_num = Voice_get_ch_num(vg->voices[i]);
        if (ch_num >= 0)
            return ch_num;
    }

    return -1;
}


static void mix_voice_signals(
        const Device_node* node,
        Voice_group* vgroup,
        Device_states* dstates,
        int thread_id,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(node != NULL);
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    const Device* node_device = Device_node_get_device(node);
    if (node_device == NULL)
        return;

    Device_thread_state* node_ts =
        Device_states_get_thread_state(dstates, thread_id, Device_get_id(node_device));

    if (Device_thread_state_get_node_state(node_ts) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_thread_state_get_node_state(node_ts) == DEVICE_NODE_STATE_VISITED);
        return;
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_REACHED);

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        if (Processor_get_voice_signals((const Processor*)node_device))
        {
            // Mix Voice signals if we have any
            bool call_mix = true;
            if ((node_device->dimpl->get_vstate_size == NULL) ||
                    (node_device->dimpl->get_vstate_size() > 0))
            {
                const uint32_t proc_id = Device_get_id(node_device);
                Voice* voice = Voice_group_get_voice_by_proc(vgroup, proc_id);
                call_mix = (voice != NULL);
            }

            if (call_mix)
                Device_thread_state_mix_voice_signals(node_ts, buf_start, buf_stop);

            // Stop recursing as we don't depend on any mixed signals
            Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_VISITED);
            return;
        }
    }

    const int last_port = Device_node_get_last_receive_port(node);
    for (int port = 0; port <= last_port; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if (send_device == NULL)
            {
                edge = edge->next;
                continue;
            }

            mix_voice_signals(
                    edge->node, vgroup, dstates, thread_id, buf_start, buf_stop);

            edge = edge->next;
        }
    }

    Device_thread_state_set_node_state(node_ts, DEVICE_NODE_STATE_VISITED);

    return;
}


void Voice_group_mix(
        Voice_group* vgroup,
        Device_states* dstates,
        int thread_id,
        const Connections* conns,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(vgroup != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(conns != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    if (buf_start >= buf_stop)
        return;

    reset_subgraph(dstates, thread_id, master);
    //Device_states_reset_node_states(dstates);
    mix_voice_signals(master, vgroup, dstates, thread_id, buf_start, buf_stop);

    return;
}


void Voice_group_deactivate_all(Voice_group* vg)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
        Voice_reset(vg->voices[i]);

    return;
}


void Voice_group_deactivate_unreachable(Voice_group* vg)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
    {
        Voice* voice = vg->voices[i];
        if (!voice->updated || !voice->state->active)
            Voice_reset(voice);
    }

    return;
}


