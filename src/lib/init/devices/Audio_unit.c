

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


#include <init/devices/Audio_unit.h>

#include <debug/assert.h>
#include <init/Au_table.h>
#include <init/Connections.h>
#include <init/devices/Au_control_vars.h>
#include <init/devices/Au_expressions.h>
#include <init/devices/Au_interface.h>
#include <init/devices/Au_streams.h>
#include <init/devices/Device.h>
#include <init/devices/Param_proc_filter.h>
#include <init/devices/Proc_table.h>
#include <init/devices/Processor.h>
#include <mathnum/common.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <player/Channel.h>
#include <player/devices/Au_state.h>
#include <string/common.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Hit_info
{
    bool existence;
    Param_proc_filter* hit_proc_filter;
} Hit_info;


static Hit_info* Hit_info_init(Hit_info* hit)
{
    rassert(hit != NULL);

    hit->existence = false;
    hit->hit_proc_filter = NULL;

    return hit;
}


static void Hit_info_deinit(Hit_info* hit)
{
    rassert(hit != NULL);

    del_Param_proc_filter(hit->hit_proc_filter);
    hit->hit_proc_filter = NULL;

    return;
}


struct Audio_unit
{
    Device parent;

    Au_type type;

    Au_interface* out_iface;
    Au_interface* in_iface;
    Connections* connections;

    Au_params params;   ///< All the Audio unit parameters that Processors need.

    Proc_table* procs;
    Au_table* au_table;

    Au_control_vars* control_vars;
    Au_streams* streams;
    Hit_info hits[KQT_HITS_MAX];
    Au_expressions* expressions;
};



//static bool Audio_unit_sync(Device* device, Device_states* dstates);

static void Audio_unit_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value);

static void Audio_unit_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel);


Audio_unit* new_Audio_unit(void)
{
    Audio_unit* au = memory_alloc_item(Audio_unit);
    if (au == NULL)
        return NULL;

    //fprintf(stderr, "New Audio unit %p\n", (void*)au);
    au->type = AU_TYPE_INVALID;
    au->out_iface = NULL;
    au->in_iface = NULL;
    au->connections = NULL;
    au->procs = NULL;
    au->au_table = NULL;
    au->control_vars = NULL;
    au->streams = NULL;
    for (int i = 0; i < KQT_HITS_MAX; ++i)
        Hit_info_init(&au->hits[i]);
    au->expressions = NULL;

    if (!Device_init(&au->parent, false))
    {
        memory_free(au);
        return NULL;
    }
    if (Au_params_init(&au->params, Device_get_id(&au->parent)) == NULL)
    {
        Device_deinit(&au->parent);
        memory_free(au);
        return NULL;
    }

    Device_set_state_creator(&au->parent, new_Au_state);
    Device_set_mixed_signals(&au->parent, true);

    Device_register_set_control_var_generic(
            &au->parent, Audio_unit_set_control_var_generic);

    Device_register_init_control_vars(&au->parent, Audio_unit_init_control_vars);

    au->out_iface = new_Au_interface();
    au->in_iface = new_Au_interface();
    au->procs = new_Proc_table(KQT_PROCESSORS_MAX);
    au->au_table = new_Au_table(KQT_AUDIO_UNITS_MAX);
    if ((au->out_iface == NULL) ||
            (au->in_iface == NULL) ||
            (au->procs == NULL) ||
            (au->au_table == NULL))
    {
        del_Audio_unit(au);
        return NULL;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_port_existence(
                &au->out_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
        Device_set_port_existence(
                &au->in_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
    }
    for (int port = 0; port < KQT_BUFFERS_MAX; ++port)
    {
        Device_set_port_existence(
                &au->out_iface->parent, DEVICE_PORT_TYPE_RECEIVE, port, true);
    }

    return au;
}


void Audio_unit_set_type(Audio_unit* au, Au_type type)
{
    rassert(au != NULL);
    rassert(type != AU_TYPE_INVALID);

    au->type = type;

    return;
}


Au_type Audio_unit_get_type(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->type;
}


Au_params* Audio_unit_get_params(Audio_unit* au)
{
    rassert(au != NULL);
    return &au->params;
}


const Processor* Audio_unit_get_proc(const Audio_unit* au, int index)
{
    rassert(au != NULL);
    rassert(index >= 0);
    rassert(index < KQT_PROCESSORS_MAX);

    return Proc_table_get_proc(au->procs, index);
}


Proc_table* Audio_unit_get_procs(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->procs;
}


const Audio_unit* Audio_unit_get_au(const Audio_unit* au, int index)
{
    rassert(au != NULL);
    rassert(au->au_table != NULL);
    rassert(index >= 0);
    rassert(index < KQT_AUDIO_UNITS_MAX);

    return Au_table_get(au->au_table, index);
}


Au_table* Audio_unit_get_au_table(Audio_unit* au)
{
    rassert(au != NULL);
    rassert(au->au_table != NULL);

    return au->au_table;
}


void Audio_unit_set_connections(Audio_unit* au, Connections* graph)
{
    rassert(au != NULL);

    if (au->connections != NULL)
        del_Connections(au->connections);
    au->connections = graph;

    return;
}


const Connections* Audio_unit_get_connections(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->connections;
}


Connections* Audio_unit_get_connections_mut(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->connections;
}


bool Audio_unit_prepare_connections(const Audio_unit* au, Device_states* states)
{
    rassert(au != NULL);
    rassert(states != NULL);

    if (au->connections == NULL)
        return true;

    return Connections_prepare(au->connections, states);
}


const Device* Audio_unit_get_input_interface(const Audio_unit* au)
{
    rassert(au != NULL);
    return &au->in_iface->parent;
}


const Device* Audio_unit_get_output_interface(const Audio_unit* au)
{
    rassert(au != NULL);
    return &au->out_iface->parent;
}


void Audio_unit_set_control_vars(Audio_unit* au, Au_control_vars* au_control_vars)
{
    rassert(au != NULL);

    del_Au_control_vars(au->control_vars);
    au->control_vars = au_control_vars;

    return;
}


void Audio_unit_set_streams(Audio_unit* au, Au_streams* au_streams)
{
    rassert(au != NULL);

    del_Au_streams(au->streams);
    au->streams = au_streams;

    return;
}


const Au_streams* Audio_unit_get_streams(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->streams;
}


void Audio_unit_set_hit_existence(Audio_unit* au, int index, bool existence)
{
    rassert(au != NULL);
    rassert(index >= 0);
    rassert(index < KQT_HITS_MAX);

    au->hits[index].existence = existence;

    return;
}


bool Audio_unit_get_hit_existence(const Audio_unit* au, int index)
{
    rassert(au != NULL);
    rassert(index >= 0);
    rassert(index < KQT_HITS_MAX);

    return au->hits[index].existence;
}


void Audio_unit_set_hit_proc_filter(Audio_unit* au, int index, Param_proc_filter* hpf)
{
    rassert(au != NULL);
    rassert(index >= 0);
    rassert(index < KQT_HITS_MAX);

    del_Param_proc_filter(au->hits[index].hit_proc_filter);
    au->hits[index].hit_proc_filter = hpf;

    return;
}


const Param_proc_filter* Audio_unit_get_hit_proc_filter(const Audio_unit* au, int index)
{
    rassert(au != NULL);
    rassert(index >= 0);
    rassert(index < KQT_HITS_MAX);

    return au->hits[index].hit_proc_filter;
}


void Audio_unit_set_expressions(Audio_unit* au, Au_expressions* expressions)
{
    rassert(au != NULL);

    del_Au_expressions(au->expressions);
    au->expressions = expressions;

    return;
}


const Au_expressions* Audio_unit_get_expressions(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->expressions;
}


static const Device* get_cv_target_device(
        const Audio_unit* au,
        const Au_control_binding_iter* iter,
        Device_control_var_mode mode)
{
    rassert(au != NULL);
    rassert(iter != NULL);

    const Device* target_dev = NULL;
    if ((mode == DEVICE_CONTROL_VAR_MODE_MIXED) &&
            (iter->target_dev_type == TARGET_DEV_AU))
        target_dev = (const Device*)Au_table_get(
                au->au_table, iter->target_dev_index);
    else if (iter->target_dev_type == TARGET_DEV_PROC)
        target_dev = (const Device*)Proc_table_get_proc(
                au->procs, iter->target_dev_index);

    return target_dev;
}


static void Audio_unit_init_control_var_generic(
        const Audio_unit* au,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        Value_type var_type)
{
    rassert(au != NULL);
    rassert(dstates != NULL);
    rassert(random != NULL);
    rassert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    rassert(var_name != NULL);

    bool carried = false;

    if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
    {
        const Channel_cv_state* cvstate = Channel_get_cv_state(channel);
        if (Channel_cv_state_is_carrying_enabled(cvstate, var_name))
        {
            const Value* carried_value =
                Channel_cv_state_get_value(cvstate, var_name);
            if (carried_value != NULL)
            {
                Value* converted = VALUE_AUTO;
                if (Value_convert(converted, carried_value, var_type))
                {
                    Audio_unit_set_control_var_generic(
                            &au->parent,
                            dstates,
                            mode,
                            random,
                            channel,
                            var_name,
                            converted);

                    carried = true;
                }
            }
        }
    }

    if (!carried)
    {
        const Value* init_value =
            Au_control_vars_get_init_value(au->control_vars, var_name);

        if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
        {
            Channel_cv_state* cvstate = Channel_get_cv_state_mut(channel);
            const bool success =
                Channel_cv_state_set_value(cvstate, var_name, init_value);
            rassert(success);
        }

        Audio_unit_set_control_var_generic(
                &au->parent, dstates, mode, random, channel, var_name, init_value);
    }

    return;
}


static void Audio_unit_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel)
{
    rassert(device != NULL);
    rassert(dstates != NULL);
    rassert(random != NULL);
    rassert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    Au_control_var_iter* var_iter =
        Au_control_var_iter_init(AU_CONTROL_VAR_ITER_AUTO, au->control_vars);
    const char* var_name = NULL;
    Value_type var_type = VALUE_TYPE_NONE;
    Au_control_var_iter_get_next_var_info(var_iter, &var_name, &var_type);
    while (var_name != NULL)
    {
        Audio_unit_init_control_var_generic(
                au, dstates, mode, random, channel, var_name, var_type);

        Au_control_var_iter_get_next_var_info(var_iter, &var_name, &var_type);
    }

    return;
}


static void Audio_unit_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value)
{
    rassert(device != NULL);
    rassert(dstates != NULL);
    rassert(random != NULL);
    rassert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    rassert(var_name != NULL);
    rassert(value != NULL);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Map our value to each bound target range and
    // call control value setter for corresponding subdevice
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init_set_generic(
            iter, au->control_vars, random, var_name, value);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if ((target_dev != NULL) && (iter->target_value.type != VALUE_TYPE_NONE))
            Device_set_control_var_generic(
                    target_dev,
                    dstates,
                    mode,
                    random,
                    channel,
                    iter->target_var_name,
                    &iter->target_value);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


void del_Audio_unit(Audio_unit* au)
{
    if (au == NULL)
        return;

    Au_params_deinit(&au->params);
    del_Connections(au->connections);
    del_Au_interface(au->in_iface);
    del_Au_interface(au->out_iface);
    del_Au_table(au->au_table);
    del_Proc_table(au->procs);
    for (int i = 0; i < KQT_HITS_MAX; ++i)
        Hit_info_deinit(&au->hits[i]);
    del_Au_expressions(au->expressions);
    del_Au_streams(au->streams);
    del_Au_control_vars(au->control_vars);
    Device_deinit(&au->parent);
    memory_free(au);

    return;
}


