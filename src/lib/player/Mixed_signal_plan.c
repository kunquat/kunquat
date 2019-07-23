

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <containers/Vector.h>
#include <debug/assert.h>
#include <init/Connections.h>
#include <init/Device_node.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Au_state.h>
#include <player/devices/Device_state.h>
#include <player/devices/Device_thread_state.h>
#include <player/Mixed_signal_plan.h>
#include <player/Work_buffer.h>

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


typedef uint32_t Task_id;


struct Mixed_signal_plan
{
    Vector* tasks;
    Device_states* dstates;
};


typedef struct Buffer_connection
{
    Work_buffer* receiver;
    const Work_buffer* sender;
} Buffer_connection;


#define MAKE_CONNECTION(recv_buf, send_buf) \
    (&(Buffer_connection){ .receiver = (recv_buf), .sender = (send_buf) })


typedef struct Mixed_signal_task_info
{
    bool is_input_required;
    int level_index;
    uint32_t device_id;
    Vector* sender_tasks;
    Vector* conns;
    uint32_t container_id;
    Vector* bypass_sender_tasks;
    Vector* bypass_conns;
} Mixed_signal_task_info;


#define MIXED_SIGNAL_TASK_INFO_AUTO     \
    (&(Mixed_signal_task_info){         \
        .is_input_required = true,      \
        .level_index = INT_MAX,         \
        .device_id = 0,                 \
        .sender_tasks = NULL,           \
        .conns = NULL,                  \
        .container_id = 0,              \
        .bypass_sender_tasks = NULL,    \
        .bypass_conns = NULL,           \
    })


static void Mixed_signal_task_info_deinit(Mixed_signal_task_info* task_info)
{
    rassert(task_info != NULL);

    del_Vector(task_info->bypass_conns);
    task_info->bypass_conns = NULL;
    del_Vector(task_info->bypass_sender_tasks);
    task_info->bypass_sender_tasks = NULL;

    del_Vector(task_info->conns);
    task_info->conns = NULL;
    del_Vector(task_info->sender_tasks);
    task_info->sender_tasks = NULL;

    return;
}


static bool Mixed_signal_task_info_init(
        Mixed_signal_task_info* task_info, uint32_t device_id, int level_index)
{
    rassert(task_info != NULL);
    rassert(level_index >= 0);

    task_info->is_input_required = true;
    task_info->level_index = level_index;
    task_info->device_id = device_id;
    task_info->conns = NULL;
    task_info->container_id = 0;
    task_info->bypass_conns = NULL;

    task_info->conns = new_Vector(sizeof(Buffer_connection));
    task_info->sender_tasks = new_Vector(sizeof(Task_id));
    if ((task_info->conns == NULL) || (task_info->sender_tasks == NULL))
        return false;

    return true;
}


static bool senders_contain_id(Vector* senders, Task_id sender_id)
{
    rassert(senders != NULL);

    const int64_t task_count = Vector_size(senders);
    for (int64_t i = 0; i < task_count; ++i)
    {
        Task_id* cur_id = Vector_get_ref(senders, i);
        if (*cur_id == sender_id)
            return true;
    }

    return false;
}


static bool Mixed_signal_task_info_add_sender_task(
        Mixed_signal_task_info* task_info, Task_id sender_id)
{
    rassert(task_info != NULL);

    if (senders_contain_id(task_info->sender_tasks, sender_id))
        return true;

    return Vector_append(task_info->sender_tasks, &sender_id);
}


static bool Mixed_signal_task_info_add_bypass_sender_task(
        Mixed_signal_task_info* task_info, Task_id sender_id)
{
    rassert(task_info != NULL);

    if (senders_contain_id(task_info->bypass_sender_tasks, sender_id))
        return true;

    return Vector_append(task_info->bypass_sender_tasks, &sender_id);
}


static bool Mixed_signal_task_info_is_empty(const Mixed_signal_task_info* task_info)
{
    rassert(task_info != NULL);
    return (task_info->is_input_required &&
            (Vector_size(task_info->sender_tasks) == 0));
}


static bool Mixed_signal_task_info_add_input(
        Mixed_signal_task_info* task_info,
        Work_buffer* recv_buf,
        const Work_buffer* send_buf)
{
    rassert(task_info != NULL);
    rassert(task_info->conns != NULL);
    rassert(recv_buf != NULL);
    rassert(send_buf != NULL);

    return Vector_append(task_info->conns, MAKE_CONNECTION(recv_buf, send_buf));
}


static bool Mixed_signal_task_info_add_bypass_input(
        Mixed_signal_task_info* task_info,
        Work_buffer* recv_buf,
        const Work_buffer* send_buf)
{
    rassert(task_info != NULL);
    rassert(recv_buf != NULL);
    rassert(send_buf != NULL);

    rassert(task_info->bypass_conns != NULL);

    return Vector_append(task_info->bypass_conns, MAKE_CONNECTION(recv_buf, send_buf));
}


static void Mixed_signal_task_info_execute(
        const Mixed_signal_task_info* task_info,
        Device_states* dstates,
        Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(task_info != NULL);
    rassert(dstates != NULL);
    rassert(wbs != NULL);
    rassert(frame_count >= 0);
    rassert(tempo > 0);

    if (frame_count == 0)
        return;

    if (task_info->container_id != 0)
    {
        // Check bypass condition
        const Au_state* container_au_state =
            (const Au_state*)Device_states_get_state(dstates, task_info->container_id);
        if (container_au_state->bypass)
        {
            //fprintf(stdout, "bypass at level %d\n", task_info->level_index);
            if (task_info->bypass_conns != NULL)
            {
                for (int i = 0; i < Vector_size(task_info->bypass_conns); ++i)
                {
                    const Buffer_connection* conn =
                        Vector_get_ref(task_info->bypass_conns, i);
                    Work_buffer_mix(conn->receiver, conn->sender, 0, frame_count);
                }
            }
            //fflush(stdout);

            return;
        }
    }

    // Copy signals between buffers
    for (int i = 0; i < Vector_size(task_info->conns); ++i)
    {
        const Buffer_connection* conn = Vector_get_ref(task_info->conns, i);
        Work_buffer_mix(conn->receiver, conn->sender, 0, frame_count);
    }

    // Process current device state
    Device_thread_state* target_ts =
        Device_states_get_thread_state(dstates, 0, task_info->device_id);
    Device_state* target_dstate = Device_states_get_state(dstates, task_info->device_id);
    Device_state_render_mixed(target_dstate, target_ts, wbs, frame_count, tempo);

    return;
}


static int64_t Mixed_signal_create_or_get_task_info(
        Mixed_signal_plan* plan, uint32_t device_id, int level_index, bool* is_new)
{
    rassert(plan != NULL);
    rassert(level_index >= 0);
    rassert(is_new != NULL);

    // Return existing task info for device if present
    const int64_t task_count = Vector_size(plan->tasks);
    for (int64_t i = 0; i < task_count; ++i)
    {
        Mixed_signal_task_info* task_info = Vector_get_ref(plan->tasks, i);
        if (task_info->device_id == device_id)
        {
            task_info->level_index = max(task_info->level_index, level_index);
            *is_new = false;
            return i;
        }
    }

    // Add new task info into tasks
    {
        Mixed_signal_task_info* task_info = MIXED_SIGNAL_TASK_INFO_AUTO;
        if (!Mixed_signal_task_info_init(task_info, device_id, level_index) ||
                !Vector_append(plan->tasks, task_info))
        {
            Mixed_signal_task_info_deinit(task_info);
            return -1;
        }
    }

    *is_new = true;

    return Vector_size(plan->tasks) - 1;
}


static bool Mixed_signal_task_info_add_au_interface(
        Mixed_signal_task_info* task_info,
        Device_thread_state* target_ts,
        Device_thread_state* source_ts)
{
    rassert(task_info != NULL);
    rassert(target_ts != NULL);
    rassert(source_ts != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* out_buf =
            Device_thread_state_get_mixed_buffer(target_ts, DEVICE_PORT_TYPE_SEND, port);

        if (out_buf != NULL)
        {
            Device_thread_state_mark_input_port_connected(source_ts, port);

            const Work_buffer* in_buf = Device_thread_state_get_mixed_buffer(
                    source_ts, DEVICE_PORT_TYPE_RECV, port);

            if (in_buf != NULL)
            {
                if (!Mixed_signal_task_info_add_sender_task(
                            task_info, source_ts->device_id))
                    return false;

                if (!Mixed_signal_task_info_add_input(task_info, out_buf, in_buf))
                    return false;
            }
        }
    }

    return true;
}


static bool Mixed_signal_plan_build_from_node(
        Mixed_signal_plan* plan,
        Device_states* dstates,
        const Device_node* node,
        int level_index,
        uint32_t container_id)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(node != NULL);
    rassert(level_index >= 0);

    const Device* node_device = Device_node_get_device(node);
    if ((node_device == NULL) || !Device_is_existent(node_device))
        return true;

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        // TODO: This would probably be a good place to mix down
        //       all the outputs of individual threads during voice processing
        if (!Device_get_mixed_signals(node_device))
            return true;
    }

    const uint32_t node_device_id = Device_get_id(node_device);

    bool is_new_task_info = false;

    int64_t task_info_index = Mixed_signal_create_or_get_task_info(
            plan, node_device_id, level_index, &is_new_task_info);
    if (task_info_index < 0)
        return false;

    if (is_new_task_info)
    {
        Mixed_signal_task_info* task_info = Vector_get_ref(plan->tasks, task_info_index);
        task_info->container_id = container_id;
    }

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        // Make sure we include mixed signal devices that don't require input signals
        if ((node_device->dimpl != NULL) &&
                (Device_impl_get_proc_type(node_device->dimpl) == Proc_type_stream))
        {
            Mixed_signal_task_info* task_info =
                Vector_get_ref(plan->tasks, task_info_index);
            task_info->is_input_required = false;
        }
    }

    Device_thread_state* recv_ts =
        Device_states_get_thread_state(dstates, 0, node_device_id);
    Device_port_type recv_port_type = DEVICE_PORT_TYPE_RECV;

    // Get depth of connections in the current device
    int cur_depth = 1;
    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_AU)
    {
        const Audio_unit* au = (const Audio_unit*)node_device;
        const Connections* au_conns = Audio_unit_get_connections(au);
        if (au_conns == NULL)
            return true;

        const uint32_t sub_container_id =
            (container_id != 0) ? container_id : node_device_id;

        const int au_conns_depth = Connections_get_depth(au_conns);

        const Device_node* au_master = Connections_get_master(au_conns);
        rassert(au_master != NULL);

        // Build in audio unit level
        const Device* in_iface = Audio_unit_get_input_interface(au);
        const Device* out_iface = Audio_unit_get_output_interface(au);

        Device_thread_state* in_iface_ts =
            Device_states_get_thread_state(dstates, 0, Device_get_id(in_iface));
        Device_thread_state* out_iface_ts =
            Device_states_get_thread_state(dstates, 0, Device_get_id(out_iface));

        // Output interface
        if (is_new_task_info && !Mixed_signal_task_info_add_au_interface(
                    Vector_get_ref(plan->tasks, task_info_index), recv_ts, out_iface_ts))
            return false;

        {
            Mixed_signal_task_info* task_info =
                Vector_get_ref(plan->tasks, task_info_index);
            task_info->container_id = sub_container_id;
        }

        // Audio unit graph
        if (!Mixed_signal_plan_build_from_node(
                    plan, dstates, au_master, level_index + 1, sub_container_id))
            return false;

        // Input interface
        {
            bool is_new_au_task_info = false;

            int64_t in_task_info_index = Mixed_signal_create_or_get_task_info(
                    plan,
                    Device_get_id(in_iface),
                    level_index + au_conns_depth,
                    &is_new_au_task_info);
            if (in_task_info_index < 0)
                return false;

            Mixed_signal_task_info* in_task_info =
                Vector_get_ref(plan->tasks, in_task_info_index);

            // NOTE: is_new_task_info is correct here, as the input interface
            //       has been touched by the recursive call above
            if (is_new_task_info && !Mixed_signal_task_info_add_au_interface(
                        in_task_info, in_iface_ts, recv_ts))
                return false;

            in_task_info->container_id = container_id;

            if (is_new_task_info && (container_id == 0))
            {
                // Set up bypass connections
                Mixed_signal_task_info* task_info =
                    Vector_get_ref(plan->tasks, task_info_index);
                task_info->bypass_sender_tasks = new_Vector(sizeof(Task_id));
                task_info->bypass_conns = new_Vector(sizeof(Buffer_connection));
                //task_info->bypass_conns = new_Vector(sizeof(Mixed_signal_connection));
                if ((task_info->bypass_sender_tasks == NULL) ||
                        (task_info->bypass_conns == NULL))
                    return false;

                for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
                {
                    Work_buffer* out_buf = Device_thread_state_get_mixed_buffer(
                            recv_ts, DEVICE_PORT_TYPE_SEND, port);

                    if (out_buf != NULL)
                    {
                        const Work_buffer* in_buf = Device_thread_state_get_mixed_buffer(
                                in_iface_ts, DEVICE_PORT_TYPE_SEND, port);

                        if (in_buf != NULL)
                        {
                            if (!Mixed_signal_task_info_add_bypass_sender_task(
                                        task_info, in_iface->id))
                                return false;

                            if (!Mixed_signal_task_info_add_bypass_input(
                                        task_info, out_buf, in_buf))
                                return false;
                        }
                    }
                }
            }

            task_info_index = in_task_info_index;
            recv_ts = in_iface_ts;
            recv_port_type = DEVICE_PORT_TYPE_SEND;
        }

        cur_depth = au_conns_depth + 2; // incl. audio unit interface bounds
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

            if (!Mixed_signal_plan_build_from_node(
                        plan,
                        dstates,
                        edge->node,
                        level_index + cur_depth,
                        container_id))
                return false;

            // Find Work buffers
            Device_thread_state* send_ts =
                Device_states_get_thread_state(dstates, 0, Device_get_id(send_device));

            const Work_buffer* send_buf = Device_thread_state_get_mixed_buffer(
                    send_ts, DEVICE_PORT_TYPE_SEND, edge->port);
            Work_buffer* recv_buf = Device_thread_state_get_mixed_buffer(
                    recv_ts, recv_port_type, port);

            if ((send_buf != NULL) && (recv_buf != NULL))
            {
                Mixed_signal_task_info* task_info =
                    Vector_get_ref(plan->tasks, task_info_index);

                if (is_new_task_info)
                {
                    if (!Mixed_signal_task_info_add_sender_task(
                                task_info, send_ts->device_id))
                        return false;

                    if (!Mixed_signal_task_info_add_input(
                                task_info, recv_buf, send_buf))
                        return false;
                }
            }

            edge = edge->next;
        }
    }

    return true;
}


#if 0
static void debug_print_tasks(const Vector* tasks)
{
    rassert(tasks != NULL);

    for (int ti = 0; ti < Vector_size(tasks); ++ti)
    {
        const Mixed_signal_task_info* tinfo = Vector_get_ref(tasks, ti);

        fprintf(stderr, "Task %d, device ID %ld, level %d",
                ti, (long)tinfo->device_id, tinfo->level_index);
        if (tinfo->container_id != 0)
            fprintf(stderr, " (au %d)", (int)tinfo->container_id);
        fprintf(stderr, ":\n");

        fprintf(stderr, "  Senders:");
        for (int i = 0; i < Vector_size(tinfo->sender_tasks); ++i)
        {
            if (i > 0)
                fprintf(stderr, ",");

            Task_id tid = UINT32_MAX;
            Vector_get(tinfo->sender_tasks, i, &tid);
            fprintf(stderr, " %ld", (long)tid);
        }
        fprintf(stderr, "\n");

#if 0
        for (int i = 0; i < Vector_size(tinfo->conns); ++i)
        {
            const Buffer_connection* conn = Vector_get_ref(tinfo->conns, i);
            fprintf(stdout, "  ####################### %p -> %p\n",
                    (const void*)conn->sender,
                    (void*)conn->receiver);
        }
#endif
    }

    return;
}
#endif


static bool Mixed_signal_plan_finalise(Mixed_signal_plan* plan)
{
    rassert(plan != NULL);

    // Sort the tasks
    {
        int64_t task_count = Vector_size(plan->tasks);
        if (task_count > 1)
        {
            Mixed_signal_task_info* tasks = Vector_get_ref(plan->tasks, 0);

            for (int new_index = 1; new_index < task_count; ++new_index)
            {
                for (int target_index = new_index - 1; target_index >= 0; --target_index)
                {
                    if (tasks[target_index].level_index >=
                            tasks[target_index + 1].level_index)
                        break;

                    Mixed_signal_task_info tmp = tasks[target_index];
                    tasks[target_index] = tasks[target_index + 1];
                    tasks[target_index + 1] = tmp;
                }
            }
        }
    }

    // Remove tasks that don't require execution
    {
        int64_t task_index = 0;
        while (task_index < Vector_size(plan->tasks))
        {
            Mixed_signal_task_info* task_info = Vector_get_ref(plan->tasks, task_index);
            if (Mixed_signal_task_info_is_empty(task_info))
            {
                // Remove connections to the empty task
                const uint32_t remove_id = task_info->device_id;
                for (int64_t li = task_index + 1; li < Vector_size(plan->tasks); ++li)
                {
                    Mixed_signal_task_info* later_task = Vector_get_ref(plan->tasks, li);
                    int64_t sender_index = 0;
                    while (sender_index < Vector_size(later_task->sender_tasks))
                    {
                        Task_id sender_id = UINT32_MAX;
                        Vector_get(later_task->sender_tasks, sender_index, &sender_id);
                        if (sender_id == remove_id)
                            Vector_remove_at(later_task->sender_tasks, sender_index);
                        else
                            ++sender_index;
                    }
                }

                Mixed_signal_task_info_deinit(task_info);
                Vector_remove_at(plan->tasks, task_index);
            }
            else
            {
                ++task_index;
            }
        }
    }

    return true;
}


static bool Mixed_signal_plan_build(
        Mixed_signal_plan* plan,
        Device_states* dstates,
        const Connections* conns)
{
    rassert(plan != NULL);
    rassert(dstates != NULL);
    rassert(conns != NULL);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);

    Device_states_reset_node_states(dstates);

    return (Mixed_signal_plan_build_from_node(plan, dstates, master, 0, 0) &&
            Mixed_signal_plan_finalise(plan));
}


Mixed_signal_plan* new_Mixed_signal_plan(
    Device_states* dstates, const Connections* conns)
{
    rassert(dstates != NULL);

    Mixed_signal_plan* plan = memory_alloc_item(Mixed_signal_plan);
    if (plan == NULL)
        return NULL;

    // Sanitise fields
    plan->tasks = NULL;
    plan->dstates = dstates;

    // Initialise
    plan->tasks = new_Vector(sizeof(Mixed_signal_task_info));
    if ((plan->tasks == NULL) || !Mixed_signal_plan_build(plan, dstates, conns))
    {
        del_Mixed_signal_plan(plan);
        return NULL;
    }

    return plan;
}


void Mixed_signal_plan_execute_all_tasks(
        Mixed_signal_plan* plan,
        Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(plan != NULL);
    rassert(wbs != NULL);
    rassert(frame_count >= 0);
    rassert(tempo > 0);

    const int64_t task_count = Vector_size(plan->tasks);
    for (int64_t task_index = 0; task_index < task_count; ++task_index)
    {
        const Mixed_signal_task_info* task_info =
            Vector_get_ref(plan->tasks, task_index);
        Mixed_signal_task_info_execute(
                task_info, plan->dstates, wbs, frame_count, tempo);
    }

    return;
}


void del_Mixed_signal_plan(Mixed_signal_plan* plan)
{
    if (plan == NULL)
        return;

    if (plan->tasks != NULL)
    {
        for (int64_t i = 0; i < Vector_size(plan->tasks); ++i)
            Mixed_signal_task_info_deinit(Vector_get_ref(plan->tasks, i));
    }

    del_Vector(plan->tasks);
    memory_free(plan);

    return;
}


