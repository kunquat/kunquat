

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


#include <containers/AAtree.h>
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_LEVELS 1024


struct Mixed_signal_plan
{
    Vector* levels;
    AAtree* build_task_infos; // TODO: remove; this is only used during initialisation

    Device_states* dstates;

    int iter_level_index;
    int iter_task_index;
};


typedef struct Level
{
    Vector* tasks;
} Level;


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
    Vector* conns;
    uint32_t container_id;
    Vector* bypass_conns;
} Mixed_signal_task_info;


#define MIXED_SIGNAL_TASK_INFO_KEY(dev_id)  \
    (&(Mixed_signal_task_info){             \
        .is_input_required = true,          \
        .level_index = -1,                  \
        .device_id = (dev_id),              \
        .conns = NULL,                      \
        .container_id = 0,                  \
        .bypass_conns = NULL,               \
    })


static void Mixed_signal_task_info_deinit(Mixed_signal_task_info* task_info)
{
    rassert(task_info != NULL);

    // NOTE: We don't own the Device states referenced
    del_Vector(task_info->bypass_conns);
    task_info->bypass_conns = NULL;
    del_Vector(task_info->conns);
    task_info->conns = NULL;

    return;
}


static void del_Mixed_signal_task_info(Mixed_signal_task_info* task_info)
{
    if (task_info == NULL)
        return;

    Mixed_signal_task_info_deinit(task_info);
    memory_free(task_info);

    return;
}


static bool Mixed_signal_task_info_init(
        Mixed_signal_task_info* task_info, uint32_t device_id, int level_index)
{
    rassert(task_info != NULL);
    rassert(level_index >= 0);
    rassert(level_index < MAX_LEVELS);

    task_info->is_input_required = true;
    task_info->level_index = level_index;
    task_info->device_id = device_id;
    task_info->conns = NULL;
    task_info->container_id = 0;
    task_info->bypass_conns = NULL;

    task_info->conns = new_Vector(sizeof(Buffer_connection));
    if (task_info->conns == NULL)
        return false;

    return true;
}


static void Mixed_signal_task_info_clear(Mixed_signal_task_info* task_info)
{
    rassert(task_info != NULL);

    task_info->conns = NULL;
    task_info->bypass_conns = NULL;

    return;
}


static bool Mixed_signal_task_info_is_empty(const Mixed_signal_task_info* task_info)
{
    rassert(task_info != NULL);
    return (task_info->is_input_required && (Vector_size(task_info->conns) == 0));
}


static bool Mixed_signal_task_info_add_input(
        Mixed_signal_task_info* task_info,
        Work_buffer* recv_buf,
        const Work_buffer* send_buf)
{
    rassert(task_info != NULL);
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


static int Mixed_signal_task_info_cmp(
        const Mixed_signal_task_info* ti1, const Mixed_signal_task_info* ti2)
{
    rassert(ti1 != NULL);
    rassert(ti2 != NULL);

    if (ti1->device_id < ti2->device_id)
        return -1;
    else if (ti1->device_id > ti2->device_id)
        return 1;
    return 0;
}


static void Level_deinit(Level* level)
{
    rassert(level != NULL);

    if (level->tasks == NULL)
        return;

    const int64_t task_count = Vector_size(level->tasks);
    for (int64_t i = 0; i < task_count; ++i)
        Mixed_signal_task_info_deinit(Vector_get_ref(level->tasks, i));

    del_Vector(level->tasks);
    level->tasks = NULL;

    return;
}


static bool Level_init(Level* level)
{
    rassert(level != NULL);

    level->tasks = NULL;

    level->tasks = new_Vector(sizeof(Mixed_signal_task_info));
    if (level->tasks == NULL)
        return false;

    return true;
}


static bool Level_is_empty(const Level* level)
{
    rassert(level != NULL);
    return (Vector_size(level->tasks) == 0);
}


static bool Level_add_task_info(Level* level, Mixed_signal_task_info* task_info)
{
    rassert(level != NULL);
    rassert(task_info != NULL);

    if (!Vector_append(level->tasks, task_info))
        return false;

    Mixed_signal_task_info_clear(task_info);

    return true;
}


static Mixed_signal_task_info* Mixed_signal_create_or_get_task_info(
        Mixed_signal_plan* plan, uint32_t device_id, int level_index, bool* is_new)
{
    rassert(plan != NULL);
    rassert(level_index >= 0);
    rassert(level_index < MAX_LEVELS);
    rassert(is_new != NULL);

    const Mixed_signal_task_info* key = MIXED_SIGNAL_TASK_INFO_KEY(device_id);
    Mixed_signal_task_info* task_info = AAtree_get_exact(plan->build_task_infos, key);
    if (task_info != NULL)
    {
        task_info->level_index = max(task_info->level_index, level_index);
        *is_new = false;
        return task_info;
    }

    task_info = memory_alloc_item(Mixed_signal_task_info);
    if (task_info == NULL)
        return NULL;

    if (!Mixed_signal_task_info_init(task_info, device_id, level_index) ||
            !AAtree_ins(plan->build_task_infos, task_info))
    {
        Mixed_signal_task_info_deinit(task_info);
        memory_free(task_info);
        return NULL;
    }

    *is_new = true;
    return task_info;
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

    Mixed_signal_task_info* task_info = Mixed_signal_create_or_get_task_info(
            plan, node_device_id, level_index, &is_new_task_info);
    if (task_info == NULL)
        return false;

    if (is_new_task_info)
        task_info->container_id = container_id;

    if (Device_node_get_type(node) == DEVICE_NODE_TYPE_PROCESSOR)
    {
        // Make sure we include mixed signal devices that don't require input signals
        if ((node_device->dimpl != NULL) &&
                (Device_impl_get_proc_type(node_device->dimpl) == Proc_type_stream))
            task_info->is_input_required = false;
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
                    task_info, recv_ts, out_iface_ts))
            return false;

        task_info->container_id = sub_container_id;

        // Audio unit graph
        if (!Mixed_signal_plan_build_from_node(
                    plan, dstates, au_master, level_index + 1, sub_container_id))
            return false;

        // Input interface
        {
            bool is_new_au_task_info = false;

            Mixed_signal_task_info* in_task_info = Mixed_signal_create_or_get_task_info(
                    plan,
                    Device_get_id(in_iface),
                    level_index + au_conns_depth,
                    &is_new_au_task_info);
            if (in_task_info == NULL)
                return false;

            // NOTE: is_new_task_info is correct here, as the input interface
            //       has been touched by the recursive call above
            if (is_new_task_info && !Mixed_signal_task_info_add_au_interface(
                        in_task_info, in_iface_ts, recv_ts))
                return false;

            in_task_info->container_id = container_id;

            if (is_new_task_info && (container_id == 0))
            {
                // Set up bypass connections
                task_info->bypass_conns = new_Vector(sizeof(Buffer_connection));
                //task_info->bypass_conns = new_Vector(sizeof(Mixed_signal_connection));
                if (task_info->bypass_conns == NULL)
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
                            if (!Mixed_signal_task_info_add_bypass_input(
                                        task_info, out_buf, in_buf))
                                return false;
                        }
                    }
                }
            }

            task_info = in_task_info;
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
                if (is_new_task_info && !Mixed_signal_task_info_add_input(
                            task_info, recv_buf, send_buf))
                    return false;
            }

            edge = edge->next;
        }
    }

    return true;
}


static bool Mixed_signal_plan_finalise(Mixed_signal_plan* plan)
{
    rassert(plan != NULL);

    // Move task info contexts from plan->build_task_infos to plan->levels
    {
        AAiter* iter = AAiter_init(AAITER_AUTO, plan->build_task_infos);
        const Mixed_signal_task_info* key = MIXED_SIGNAL_TASK_INFO_KEY(0);
        Mixed_signal_task_info* task_info = AAiter_get_at_least(iter, key);
        while (task_info != NULL)
        {
            if (!Mixed_signal_task_info_is_empty(task_info))
            {
                while (task_info->level_index >= Vector_size(plan->levels))
                {
                    Level* level = &(Level){ .tasks = NULL };
                    if (!Level_init(level) || !Vector_append(plan->levels, level))
                    {
                        Level_deinit(level);
                        return false;
                    }
                }

                Level* level = Vector_get_ref(plan->levels, task_info->level_index);

                if (!Level_add_task_info(level, task_info))
                    return false;
            }

            AAtree_remove(plan->build_task_infos, task_info);
            del_Mixed_signal_task_info(task_info);

            // Reinitialise iterator as it is no longer valid
            AAiter_init(iter, plan->build_task_infos);
            task_info = AAiter_get_at_least(iter, key);
        }
    }

    del_AAtree(plan->build_task_infos);
    plan->build_task_infos = NULL;

    // Remove empty levels
    int64_t level_index = 0;
    while (level_index < Vector_size(plan->levels))
    {
        Level* level = Vector_get_ref(plan->levels, level_index);
        if (Level_is_empty(level))
        {
            Level_deinit(Vector_get_ref(plan->levels, level_index));
            Vector_remove_at(plan->levels, level_index);
        }
        else
        {
            ++level_index;
        }
    }

#if 0
    for (int li = Vector_size(plan->levels) - 1; li >= 0; --li)
    {
        const Level* level = Vector_get_ref(plan->levels, li);
        if (level == NULL)
            continue;

        fprintf(stdout, "Level %d:\n", li);
        for (int ti = 0; ti < Vector_size(level->tasks); ++ti)
        {
            const Mixed_signal_task_info* tinfo = Vector_get_ref(level->tasks, ti);
            rassert(tinfo != NULL);

            fprintf(stdout, " Task %d", ti);
            if (tinfo->container_id != 0)
                fprintf(stdout, " (au %d)", (int)tinfo->container_id);
            fprintf(stdout, ":\n");
            for (int i = 0; i < Vector_size(tinfo->conns); ++i)
            {
                const Mixed_signal_connection* conn = Vector_get_ref(tinfo->conns, i);
                fprintf(stdout, "  ####################### %p -> %p\n",
                        (const void*)conn->send_buf,
                        (void*)conn->recv_buf);
            }
        }

        fflush(stdout);
    }
#endif

    Mixed_signal_plan_reset(plan);

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
    plan->levels = NULL;
    plan->build_task_infos = NULL;
    plan->dstates = dstates;
    plan->iter_level_index = -1;
    plan->iter_task_index = 0;

    // Initialise
    plan->levels = new_Vector(sizeof(Level));
    plan->build_task_infos = new_AAtree(
            (AAtree_item_cmp*)Mixed_signal_task_info_cmp,
            (AAtree_item_destroy*)del_Mixed_signal_task_info);
    if ((plan->levels == NULL) ||
            (plan->build_task_infos == NULL) ||
            !Mixed_signal_plan_build(plan, dstates, conns))
    {
        del_Mixed_signal_plan(plan);
        return NULL;
    }

    return plan;
}


int Mixed_signal_plan_get_level_count(const Mixed_signal_plan* plan)
{
    rassert(plan != NULL);
    return (int)Vector_size(plan->levels);
}


void Mixed_signal_plan_reset(Mixed_signal_plan* plan)
{
    rassert(plan != NULL);

    plan->iter_level_index = (int)Vector_size(plan->levels) - 1;
    plan->iter_task_index = 0;

    return;
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

    const int64_t level_count = Vector_size(plan->levels);
    for (int64_t level_index = level_count - 1; level_index >= 0; --level_index)
    {
        const Level* level = Vector_get_ref(plan->levels, level_index);

        const int64_t task_count = Vector_size(level->tasks);
        for (int64_t task_index = 0; task_index < task_count; ++task_index)
        {
            const Mixed_signal_task_info* task_info =
                Vector_get_ref(level->tasks, task_index);
            Mixed_signal_task_info_execute(
                    task_info, plan->dstates, wbs, frame_count, tempo);
        }
    }

    return;
}


void del_Mixed_signal_plan(Mixed_signal_plan* plan)
{
    if (plan == NULL)
        return;

    if (plan->levels != NULL)
    {
        for (int64_t i = 0; i < Vector_size(plan->levels); ++i)
            Level_deinit(Vector_get_ref(plan->levels, i));
    }

    del_Vector(plan->levels);
    del_AAtree(plan->build_task_infos);
    memory_free(plan);

    return;
}


