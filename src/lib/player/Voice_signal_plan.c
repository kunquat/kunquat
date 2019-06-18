

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018-2019
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
#include <player/devices/Device_state.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/Voice.h>
#include <player/Voice_group.h>
#include <player/Work_buffer.h>
#include <player/Work_buffer_conn_rules.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef int16_t Task_index;


typedef struct Voice_signal_task_info
{
    uint32_t device_id;
    Vector* sender_tasks;
    Vector* buf_conns;
    uint32_t is_connected_to_mixed : 1;
    uint32_t is_processed : 1;
} Voice_signal_task_info;


static void Voice_signal_task_info_deinit(Voice_signal_task_info* task_info)
{
    rassert(task_info != NULL);

    del_Vector(task_info->sender_tasks);
    task_info->sender_tasks = NULL;

    del_Vector(task_info->buf_conns);
    task_info->buf_conns = NULL;

    return;
}


static bool Voice_signal_task_info_init(
        Voice_signal_task_info* task_info, uint32_t device_id)
{
    rassert(task_info != NULL);

    task_info->device_id = device_id;
    task_info->is_connected_to_mixed = false;
    task_info->is_processed = false;

    task_info->sender_tasks = new_Vector(sizeof(Task_index));
    task_info->buf_conns = new_Vector(sizeof(Work_buffer_conn_rules));
    if ((task_info->sender_tasks == NULL) || (task_info->buf_conns == NULL))
        return false;

    return true;
}


static bool Voice_signal_task_info_add_sender_task(
        Voice_signal_task_info* task_info, Task_index sender_index)
{
    rassert(task_info != NULL);

    Vector* senders = task_info->sender_tasks;

    const uint16_t task_count = (uint16_t)Vector_size(senders);
    for (uint16_t i = 0; i < task_count; ++i)
    {
        Task_index cur_index = -1;
        Vector_get(senders, i, &cur_index);
        if (cur_index == sender_index)
            return true;
    }

    return Vector_append(task_info->sender_tasks, &sender_index);
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

    return Vector_append(task_info->buf_conns, rules);
}


static void Voice_signal_task_info_merge_connections(Voice_signal_task_info* task_info)
{
    rassert(task_info != NULL);

    int cur_size = (int)Vector_size(task_info->buf_conns);
    for (int di = 0; di < cur_size - 1; ++di)
    {
        Work_buffer_conn_rules* dest_rules = Vector_get_ref(task_info->buf_conns, di);

        for (int si = di + 1; si < cur_size; ++si)
        {
            const Work_buffer_conn_rules* src_rules =
                Vector_get_ref(task_info->buf_conns, si);
            if (Work_buffer_conn_rules_try_merge(dest_rules, dest_rules, src_rules))
            {
                Vector_remove_at(task_info->buf_conns, si);
                --cur_size;
                break;
            }
        }
    }

    return;
}


static void Voice_signal_task_info_invalidate_buffers(
        const Voice_signal_task_info* task_info,
        Device_states* dstates,
        int thread_id,
        int32_t frame_count)
{
    rassert(task_info != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(frame_count >= 0);

    Device_thread_state* dev_ts =
        Device_states_get_thread_state(dstates, thread_id, task_info->device_id);
    //Device_thread_state_clear_voice_buffers(dev_ts, 0, frame_count);
    Device_thread_state_invalidate_voice_buffers(dev_ts);

    return;
}


static int32_t Voice_signal_task_info_execute(
        Voice_signal_task_info* task_info,
        Vector* tasks,
        Device_states* dstates,
        int thread_id,
        Voice_group* vgroup,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo,
        bool* is_task_active)
{
    rassert(task_info != NULL);
    rassert(tasks != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(vgroup != NULL);
    rassert(wbs != NULL);
    rassert(frame_count >= 0);
    rassert(tempo > 0);
    rassert(is_task_active != NULL);

    if (task_info->is_processed)
        return 0;

    int32_t keep_alive_stop = 0;

    // Execute dependencies
    const int64_t sender_count = Vector_size(task_info->sender_tasks);
    for (int64_t i = 0; i < sender_count; ++i)
    {
        Task_index sender_index = -1;
        Vector_get(task_info->sender_tasks, i, &sender_index);
        Voice_signal_task_info* sender_task_info = Vector_get_ref(tasks, sender_index);

        bool is_sender_active = false;
        const int32_t sender_keep_alive_stop = Voice_signal_task_info_execute(
                sender_task_info,
                tasks,
                dstates,
                thread_id,
                vgroup,
                wbs,
                frame_count,
                tempo,
                &is_sender_active);

        keep_alive_stop = max(keep_alive_stop, sender_keep_alive_stop);
    }

    // Mix signals to input buffers
    const int64_t conn_count = Vector_size(task_info->buf_conns);
    for (int64_t i = 0; i < conn_count; ++i)
    {
        const Work_buffer_conn_rules* rules = Vector_get_ref(task_info->buf_conns, i);
        Work_buffer_conn_rules_mix(rules, frame_count);
    }

    bool active = false;

    // Process current processor state
    {
        Voice* voice = NULL;
        bool call_render = true;

        Device_state* dstate = Device_states_get_state(dstates, task_info->device_id);
        const Device_impl* dimpl = dstate->device->dimpl;
        rassert(dimpl != NULL);

        const Proc_state* proc_state = (Proc_state*)dstate;

        if (Proc_state_needs_vstate(proc_state))
        {
            voice = Voice_group_get_voice_by_proc(vgroup, task_info->device_id);
            call_render = (voice != NULL) && (voice->prio != VOICE_PRIO_INACTIVE);
        }

        if (call_render)
        {
            const int32_t voice_keep_alive_stop = Voice_render(
                    voice,
                    task_info->device_id,
                    dstates,
                    thread_id,
                    wbs,
                    frame_count,
                    tempo);

            keep_alive_stop = max(keep_alive_stop, voice_keep_alive_stop);

            active = true;
        }
    }

    *is_task_active = active;

    task_info->is_processed = true;

    return keep_alive_stop;
}


static void Voice_signal_task_info_mix(
        const Voice_signal_task_info* task_info,
        Device_states* dstates,
        int thread_id,
        int32_t keep_alive_stop,
        int32_t frame_offset,
        int32_t frame_count)
{
    rassert(task_info != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(frame_offset >= 0);
    rassert(frame_count >= 0);

    if (task_info->is_connected_to_mixed)
    {
        Device_thread_state* dev_ts =
            Device_states_get_thread_state(dstates, thread_id, task_info->device_id);
        Device_thread_state_mix_voice_signals(
                dev_ts, 0, keep_alive_stop, frame_offset, frame_count);
    }

    return;
}


struct Voice_signal_plan
{
    Vector* roots;
    Vector* tasks[KQT_THREADS_MAX];
};


static bool Voice_signal_plan_build_from_node(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        const Device_node* node,
        bool is_parent_mixed,
        Task_index* created_task_index)
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

    Vector* tasks = plan->tasks[thread_id];

    if (Device_thread_state_get_node_state(recv_ts) > DEVICE_NODE_STATE_NEW)
    {
        rassert(Device_thread_state_get_node_state(recv_ts) == DEVICE_NODE_STATE_VISITED);

        // Update existing task info and report its index
        const int64_t task_count = Vector_size(tasks);
        for (int64_t i = 0; i < task_count; ++i)
        {
            Voice_signal_task_info* task_info = Vector_get_ref(tasks, i);
            if (task_info->device_id == node_device_id)
            {
                if (is_parent_mixed)
                    task_info->is_connected_to_mixed = true;

                if (created_task_index != NULL)
                    *created_task_index = (Task_index)i;
            }
        }

        return true;
    }

    Device_thread_state_set_node_state(recv_ts, DEVICE_NODE_STATE_REACHED);

    const bool use_voice_signals =
        (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR) &&
        !Device_get_mixed_signals(node_device);

    Task_index cur_task_index = -1;
    if (use_voice_signals)
    {
        Voice_signal_task_info new_task_info;
        rassert(Vector_size(tasks) <= (int64_t)UINT16_MAX);
        cur_task_index = (Task_index)Vector_size(tasks);

        if (!Voice_signal_task_info_init(&new_task_info, node_device_id) ||
                !Vector_append(tasks, &new_task_info))
        {
            Voice_signal_task_info_deinit(&new_task_info);
            return false;
        }

        Voice_signal_task_info* task_info = Vector_get_ref(tasks, cur_task_index);
        task_info->is_connected_to_mixed = is_parent_mixed;
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

            Task_index sender_task_index = -1;
            if (!Voice_signal_plan_build_from_node(
                        plan,
                        dstates,
                        thread_id,
                        edge->node,
                        !use_voice_signals,
                        &sender_task_index))
                return false;

            const bool sender_is_voice_proc =
                (Device_node_get_type(edge->node) == DEVICE_NODE_TYPE_PROCESSOR) &&
                !Device_get_mixed_signals(send_device);
            if (!sender_is_voice_proc && use_voice_signals)
            {
                edge = edge->next;
                continue;
            }

            if (use_voice_signals)
            {
                // Find Work buffers
                Device_thread_state* send_ts = Device_states_get_thread_state(
                        dstates, thread_id, Device_get_id(send_device));
                rassert(send_ts != NULL);

                int send_sub_index = 0;
                const Work_buffer* send_buf = Device_thread_state_get_voice_buffer(
                        send_ts, DEVICE_PORT_TYPE_SEND, edge->port, &send_sub_index);
                int recv_sub_index = 0;
                Work_buffer* recv_buf = Device_thread_state_get_voice_buffer(
                        recv_ts, DEVICE_PORT_TYPE_RECV, port, &recv_sub_index);

                if ((send_buf != NULL) && (recv_buf != NULL) && (sender_task_index >= 0))
                {
                    Voice_signal_task_info* task_info =
                        Vector_get_ref(tasks, cur_task_index);
                    if (!Voice_signal_task_info_add_sender_task(
                                task_info, sender_task_index))
                        return false;

                    if (!Voice_signal_task_info_add_input(
                                task_info,
                                recv_buf,
                                recv_sub_index,
                                send_buf,
                                send_sub_index))
                        return false;
                }
            }

            edge = edge->next;
        }
    }

    if ((cur_task_index >= 0) && (created_task_index != NULL))
        *created_task_index = cur_task_index;

    Device_thread_state_set_node_state(recv_ts, DEVICE_NODE_STATE_VISITED);

    return true;
}


static void Voice_signal_plan_finalise(Voice_signal_plan* plan, int thread_id)
{
    rassert(plan != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);

    Vector* tasks = plan->tasks[thread_id];
    rassert(tasks != NULL);

    for (int i = 0; i < Vector_size(tasks); ++i)
    {
        Voice_signal_task_info* task_info = Vector_get_ref(tasks, i);
        rassert(task_info != NULL);
        Voice_signal_task_info_merge_connections(task_info);
    }

    return;
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
    if (!Voice_signal_plan_build_from_node(plan, dstates, thread_id, master, true, NULL))
        return false;

    Voice_signal_plan_finalise(plan, thread_id);

    return true;
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

    plan->roots = NULL;
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        plan->tasks[i] = NULL;

    plan->roots = new_Vector(sizeof(Task_index));
    if (plan->roots == NULL)
    {
        del_Voice_signal_plan(plan);
        return NULL;
    }

    for (int thread_id = 0; thread_id < thread_count; ++thread_id)
    {
        plan->tasks[thread_id] = new_Vector(sizeof(Voice_signal_task_info));
        if ((plan->tasks[thread_id] == NULL) ||
                !Voice_signal_plan_build(plan, dstates, thread_id, conns))
        {
            del_Voice_signal_plan(plan);
            return NULL;
        }
    }

    const int64_t task_count = Vector_size(plan->tasks[0]);
    for (int64_t i = 0; i < task_count; ++i)
    {
        const Voice_signal_task_info* task_info = Vector_get_ref(plan->tasks[0], i);
        if (task_info->is_connected_to_mixed)
        {
            const Task_index task_index = (Task_index)i;
            if (!Vector_append(plan->roots, &task_index))
            {
                del_Voice_signal_plan(plan);
                return NULL;
            }
        }
    }

    for (int64_t i = 0; i < task_count; ++i)
    {
        const Voice_signal_task_info* task_info = Vector_get_ref(plan->tasks[0], i);
        Proc_state* proc_state =
            (Proc_state*)Device_states_get_state(dstates, task_info->device_id);
        proc_state->is_voice_connected_to_mixed = task_info->is_connected_to_mixed;
    }

    return plan;
}


int32_t Voice_signal_plan_execute(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        Voice_group* vgroup,
        const Work_buffers* wbs,
        int32_t frame_count,
        int32_t frame_offset,
        int32_t total_frame_count,
        double tempo,
        bool enable_mixing)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(vgroup != NULL);
    rassert(wbs != NULL);
    rassert(frame_count >= 0);
    rassert(frame_offset >= 0);
    rassert(total_frame_count >= frame_count);
    rassert(frame_count + frame_offset <= total_frame_count);
    rassert(tempo > 0);

    int32_t keep_alive_stop = 0;

    bool any_active_tasks_connected_to_mixed = false;

    Vector* tasks = plan->tasks[thread_id];
    rassert(tasks != NULL);

    const int64_t task_count = Vector_size(tasks);
    for (int64_t i = 0; i < task_count; ++i)
    {
        Voice_signal_task_info* task_info = Vector_get_ref(tasks, i);
        Voice_signal_task_info_invalidate_buffers(
                task_info, dstates, thread_id, frame_count);
        task_info->is_processed = false;
    }

    const int64_t root_count = Vector_size(plan->roots);
    for (int64_t i = 0; i < root_count; ++i)
    {
        Task_index root_index = -1;
        Vector_get(plan->roots, i, &root_index);

        Voice_signal_task_info* task_info = Vector_get_ref(tasks, root_index);

        bool is_task_active = false;

        const int32_t task_keep_alive_stop = Voice_signal_task_info_execute(
                task_info,
                tasks,
                dstates,
                thread_id,
                vgroup,
                wbs,
                frame_count,
                tempo,
                &is_task_active);

        if (is_task_active)
            any_active_tasks_connected_to_mixed = true;

        keep_alive_stop = max(keep_alive_stop, task_keep_alive_stop);
    }

    if (enable_mixing)
    {
        for (int64_t i = 0; i < root_count; ++i)
        {
            Task_index root_index = -1;
            Vector_get(plan->roots, i, &root_index);

            Voice_signal_task_info* task_info = Vector_get_ref(tasks, root_index);
            Voice_signal_task_info_mix(
                    task_info,
                    dstates,
                    thread_id,
                    keep_alive_stop,
                    frame_offset,
                    total_frame_count);
        }
    }

    if (!any_active_tasks_connected_to_mixed)
        Voice_group_deactivate_all(vgroup);

    return keep_alive_stop;
}


void del_Voice_signal_plan(Voice_signal_plan* plan)
{
    if (plan == NULL)
        return;

    for (int thread_id = 0; thread_id < KQT_THREADS_MAX; ++thread_id)
    {
        if (plan->tasks[thread_id] != NULL)
        {
            Vector* tasks = plan->tasks[thread_id];
            for (int i = 0; i < Vector_size(tasks); ++i)
                Voice_signal_task_info_deinit(Vector_get_ref(tasks, i));

            del_Vector(plan->tasks[thread_id]);
        }
    }

    del_Vector(plan->roots);

    memory_free(plan);

    return;
}


