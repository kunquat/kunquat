

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice_signal_plan.h>

#include <containers/Etable.h>
#include <containers/Vector.h>
#include <debug/assert.h>
#include <init/Connections.h>
#include <init/Device_node.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/Voice_group.h>
#include <player/Work_buffer.h>
#include <player/Work_buffer_conn_rules.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct Voice_signal_task_info
{
    uint32_t device_id;
    Vector* conns;
    bool is_connected_to_mixed;
} Voice_signal_task_info;


static void del_Voice_signal_task_info(Voice_signal_task_info* task_info)
{
    if (task_info == NULL)
        return;

    del_Vector(task_info->conns);
    memory_free(task_info);

    return;
}


static Voice_signal_task_info* new_Voice_signal_task_info(uint32_t device_id)
{
    Voice_signal_task_info* task_info = memory_alloc_item(Voice_signal_task_info);
    if (task_info == NULL)
        return NULL;

    task_info->device_id = device_id;
    task_info->conns = NULL;
    task_info->is_connected_to_mixed = false;

    task_info->conns = new_Vector(sizeof(Work_buffer_conn_rules));
    if (task_info->conns == NULL)
    {
        del_Voice_signal_task_info(task_info);
        return NULL;
    }

    return task_info;
}


static bool Voice_signal_task_info_add_input(
        Voice_signal_task_info* task_info,
        Work_buffer* recv_buf,
        int recv_sub_index,
        const Work_buffer* send_buf,
        int send_sub_index)
{
    rassert(task_info != NULL);
    rassert(recv_buf != NULL);
    rassert(recv_sub_index >= 0);
    rassert(recv_sub_index < Work_buffer_get_sub_count(recv_buf));
    rassert(send_buf != NULL);
    rassert(send_sub_index >= 0);
    rassert(send_sub_index < Work_buffer_get_sub_count(send_buf));

    const Work_buffer_conn_rules* rules = Work_buffer_conn_rules_init(
            WORK_BUFFER_CONN_RULES_AUTO,
            recv_buf,
            recv_sub_index,
            send_buf,
            send_sub_index);

    return Vector_append(task_info->conns, rules);
}


static void Voice_signal_task_info_invalidate_buffers(
        const Voice_signal_task_info* task_info,
        Device_states* dstates,
        int thread_id,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(task_info != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    Device_thread_state* dev_ts =
        Device_states_get_thread_state(dstates, thread_id, task_info->device_id);
    //Device_thread_state_clear_voice_buffers(dev_ts, buf_start, buf_stop);
    Device_thread_state_invalidate_voice_buffers(dev_ts);

    return;
}


static int32_t Voice_signal_task_info_execute(
        const Voice_signal_task_info* task_info,
        Device_states* dstates,
        int thread_id,
        Voice_group* vgroup,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(task_info != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(vgroup != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);
    rassert(tempo > 0);

    // Mix signals to input buffers
    for (int i = 0; i < Vector_size(task_info->conns); ++i)
    {
        const Work_buffer_conn_rules* rules = Vector_get_ref(task_info->conns, i);
        Work_buffer_conn_rules_mix(rules, buf_start, buf_stop);
    }

    int keep_alive_stop = buf_start;

    // Process current processor state
    {
        Voice* voice = NULL;
        bool call_render = true;

        Device_state* dstate = Device_states_get_state(dstates, task_info->device_id);
        const Device_impl* dimpl = dstate->device->dimpl;
        rassert(dimpl != NULL);

        if ((dimpl->get_vstate_size == NULL) || (dimpl->get_vstate_size() > 0))
        {
            voice = Voice_group_get_voice_by_proc(vgroup, task_info->device_id);
            call_render = (voice != NULL);
        }

        if (call_render)
        {
            keep_alive_stop = Voice_render(
                    voice,
                    task_info->device_id,
                    dstates,
                    thread_id,
                    wbs,
                    buf_start,
                    buf_stop,
                    tempo);
        }
    }

    return keep_alive_stop;
}


static void Voice_signal_task_info_mix(
        const Voice_signal_task_info* task_info,
        Device_states* dstates,
        int thread_id,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(task_info != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    if (task_info->is_connected_to_mixed)
    {
        Device_thread_state* dev_ts =
            Device_states_get_thread_state(dstates, thread_id, task_info->device_id);
        Device_thread_state_mix_voice_signals(dev_ts, buf_start, buf_stop);
    }

    return;
}


struct Voice_signal_plan
{
    int task_count;
    Etable* tasks[KQT_THREADS_MAX];
};


static bool Voice_signal_plan_build_from_node(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        const Device_node* node)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if ((node_device == NULL) || !Device_is_existent(node_device))
        return true;

    const uint32_t node_device_id = Device_get_id(node_device);
    Device_thread_state* recv_ts =
        Device_states_get_thread_state(dstates, thread_id, node_device_id);

    if (Device_thread_state_get_node_state(recv_ts) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_thread_state_get_node_state(recv_ts) == DEVICE_NODE_STATE_VISITED);
        return true;
    }

    Device_thread_state_set_node_state(recv_ts, DEVICE_NODE_STATE_REACHED);

    const bool use_voice_signals =
        (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR) &&
        !Device_get_mixed_signals(node_device);

    Voice_signal_task_info* task_info = NULL;
    if (use_voice_signals)
    {
        task_info = new_Voice_signal_task_info(node_device_id);
        if (task_info == NULL)
            return false;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

        if (edge != NULL)
            Device_thread_state_mark_input_port_connected(recv_ts, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if ((send_device == NULL) || !Device_is_existent(send_device))
            {
                edge = edge->next;
                continue;
            }

            if (!Voice_signal_plan_build_from_node(
                        plan,
                        dstates,
                        thread_id,
                        edge->node))
            {
                del_Voice_signal_task_info(task_info);
                return false;
            }

            if (use_voice_signals)
            {
                // Find Work buffers
                Device_thread_state* send_ts = Device_states_get_thread_state(
                        dstates, thread_id, Device_get_id(send_device));

                int send_sub_index = 0;
                const Work_buffer* send_buf = Device_thread_state_get_voice_buffer(
                        send_ts, DEVICE_PORT_TYPE_SEND, edge->port, &send_sub_index);
                int recv_sub_index = 0;
                Work_buffer* recv_buf = Device_thread_state_get_voice_buffer(
                        recv_ts, DEVICE_PORT_TYPE_RECV, port, &recv_sub_index);

                if ((send_buf != NULL) && (recv_buf != NULL))
                {
                    if (!Voice_signal_task_info_add_input(
                                task_info,
                                recv_buf,
                                recv_sub_index,
                                send_buf,
                                send_sub_index))
                    {
                        del_Voice_signal_task_info(task_info);
                        return false;
                    }
                }
            }

            edge = edge->next;
        }
    }

    if (task_info != NULL)
    {
        rassert(Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR);

        if (!Etable_set(plan->tasks[thread_id], plan->task_count, task_info))
        {
            del_Voice_signal_task_info(task_info);
            return false;
        }

        ++plan->task_count;
    }

    Device_thread_state_set_node_state(recv_ts, DEVICE_NODE_STATE_VISITED);

    return true;
}


static Voice_signal_task_info* Voice_signal_plan_find_task_info(
        Voice_signal_plan* plan, int thread_id, uint32_t device_id)
{
    rassert(plan != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);

    Etable* tasks = plan->tasks[thread_id];
    rassert(tasks != NULL);

    for (int i = 0; i < plan->task_count; ++i)
    {
        Voice_signal_task_info* task_info = Etable_get(tasks, i);
        if (task_info->device_id == device_id)
            return task_info;
    }

    return NULL;
}


static void Voice_signal_plan_mark_connections_to_mixed(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        const Device_node* node,
        bool is_parent_mixed)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(node != NULL);

    const Device* node_device = Device_node_get_device(node);
    if ((node_device == NULL) || !Device_is_existent(node_device))
        return;

    const uint32_t node_device_id = Device_get_id(node_device);
    Device_thread_state* node_ts =
        Device_states_get_thread_state(dstates, thread_id, node_device_id);

    const bool is_current_mixed = Device_get_mixed_signals(node_device);

    if ((Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR) &&
            !Device_get_mixed_signals(node_device))
    {
        rassert(is_parent_mixed);

        if (Device_thread_state_get_node_state(node_ts) > DEVICE_NODE_STATE_NEW)
        {
            rassert(Device_thread_state_get_node_state(node_ts) ==
                    DEVICE_NODE_STATE_VISITED);
            return;
        }

        // TODO: find task info and set connected to mixed
        Voice_signal_task_info* task_info =
            Voice_signal_plan_find_task_info(plan, thread_id, node_device_id);
        rassert(task_info != NULL);
        task_info->is_connected_to_mixed = true;

        return;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const Connection* edge = Device_node_get_received(node, port);

        while (edge != NULL)
        {
            const Device* send_device = Device_node_get_device(edge->node);
            if ((send_device == NULL) || !Device_is_existent(send_device))
            {
                edge = edge->next;
                continue;
            }

            Voice_signal_plan_mark_connections_to_mixed(
                    plan, dstates, thread_id, edge->node, is_current_mixed);

            edge = edge->next;
        }
    }
}


static bool Voice_signal_plan_finalise(Voice_signal_plan* plan)
{
    rassert(plan != NULL);

    // TODO: Optimise connections

    return true;
}


static bool Voice_signal_plan_build(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        const Connections* conns)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(conns != NULL);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);

    Device_states_reset_node_states(dstates);
    if (!Voice_signal_plan_build_from_node(plan, dstates, thread_id, master))
        return false;

    const bool is_parent_mixed = true;

    Device_states_reset_node_states(dstates);
    Voice_signal_plan_mark_connections_to_mixed(
            plan, dstates, thread_id, master, is_parent_mixed);

    return Voice_signal_plan_finalise(plan);
}


Voice_signal_plan* new_Voice_signal_plan(
        Device_states* dstates, int thread_count, const Connections* conns)
{
    rassert(dstates != NULL);
    rassert(thread_count > 0);
    rassert(thread_count <= KQT_THREADS_MAX);
    rassert(conns != NULL);

    Voice_signal_plan* plan = memory_alloc_item(Voice_signal_plan);
    if (plan == NULL)
        return NULL;

    plan->task_count = 0;
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        plan->tasks[i] = NULL;

    for (int thread_id = 0; thread_id < thread_count; ++thread_id)
    {
        Device_states_reset_node_states(dstates);

        // FIXME: used as a counter by Voice_signal_plan_build; find a cleaner solution
        plan->task_count = 0;

        plan->tasks[thread_id] = new_Etable(
                KQT_PROCESSORS_MAX, (void(*)(void*))del_Voice_signal_task_info);
        if ((plan->tasks[thread_id] == NULL) ||
                !Voice_signal_plan_build(plan, dstates, thread_id, conns))
        {
            del_Voice_signal_plan(plan);
            return NULL;
        }
    }

    return plan;
}


int32_t Voice_signal_plan_execute(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        Voice_group* vgroup,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo,
        bool enable_mixing)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(vgroup != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);
    rassert(tempo > 0);

    int32_t keep_alive_stop = buf_start;

    Etable* tasks = plan->tasks[thread_id];
    rassert(tasks != NULL);

    for (int i = 0; i < plan->task_count; ++i)
    {
        const Voice_signal_task_info* task_info = Etable_get(tasks, i);
        rassert(task_info != NULL);
        Voice_signal_task_info_invalidate_buffers(
                task_info, dstates, thread_id, buf_start, buf_stop);
    }

    for (int i = 0; i < plan->task_count; ++i)
    {
        const Voice_signal_task_info* task_info = Etable_get(tasks, i);

        const int32_t task_keep_alive_stop = Voice_signal_task_info_execute(
                task_info,
                dstates,
                thread_id,
                vgroup,
                wbs,
                buf_start,
                buf_stop,
                tempo);

        keep_alive_stop = max(keep_alive_stop, task_keep_alive_stop);
    }

    if (enable_mixing)
    {
        for (int i = 0; i < plan->task_count; ++i)
        {
            const Voice_signal_task_info* task_info = Etable_get(tasks, i);
            Voice_signal_task_info_mix(
                    task_info, dstates, thread_id, buf_start, keep_alive_stop);
        }
    }

    return keep_alive_stop;
}


void del_Voice_signal_plan(Voice_signal_plan* plan)
{
    if (plan == NULL)
        return;

    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        del_Etable(plan->tasks[i]);

    memory_free(plan);

    return;
}


