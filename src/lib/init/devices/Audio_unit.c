

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2021
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
#include <init/devices/Au_event_map.h>
#include <init/devices/Au_expressions.h>
#include <init/devices/Au_interface.h>
#include <init/devices/Au_streams.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Param_proc_filter.h>
#include <init/devices/Proc_table.h>
#include <init/devices/Proc_type.h>
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

    Au_event_map* event_map;
    Au_streams* streams;
    Hit_info hits[KQT_HITS_MAX];
    Au_expressions* expressions;
};


//static bool Audio_unit_sync(Device* device, Device_states* dstates);


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
    au->event_map = NULL;
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

    // TODO: Make these existences match the outside interface
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_port_existence(
                &au->out_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
        Device_set_port_existence(
                &au->in_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_port_existence(
                &au->out_iface->parent, DEVICE_PORT_TYPE_RECV, port, true);
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


int32_t Audio_unit_get_voice_wb_size(const Audio_unit* au, int32_t audio_rate)
{
    rassert(au != NULL);
    rassert(audio_rate > 0);

    int32_t voice_wb_size = 0;

    for (int au2_i = 0; au2_i < KQT_AUDIO_UNITS_MAX; ++au2_i)
    {
        const Audio_unit* au2 = Audio_unit_get_au(au, au2_i);
        if (au2 != NULL)
        {
            const int32_t au2_voice_wb_size =
                Audio_unit_get_voice_wb_size(au2, audio_rate);
            voice_wb_size = max(voice_wb_size, au2_voice_wb_size);
        }
    }

    for (int pi = 0; pi < KQT_PROCESSORS_MAX; ++pi)
    {
        const Processor* proc = Audio_unit_get_proc(au, pi);
        if (proc != NULL)
        {
            const Device_impl* dimpl = Device_get_impl((const Device*)proc);
            if (dimpl != NULL)
            {
                const int32_t dimpl_voice_wb_size =
                    Device_impl_get_voice_wb_size(dimpl, audio_rate);
                voice_wb_size = max(voice_wb_size, dimpl_voice_wb_size);
            }
        }
    }

    return voice_wb_size;
}


void Audio_unit_set_event_map(Audio_unit* au, Au_event_map* map)
{
    rassert(au != NULL);

    del_Au_event_map(au->event_map);
    au->event_map = map;

    return;
}


const Au_event_map* Audio_unit_get_event_map(const Audio_unit* au)
{
    rassert(au != NULL);
    return au->event_map;
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


bool Audio_unit_validate_streams(
        const Audio_unit* au, char error_msg[128 + KQT_VAR_NAME_MAX + 1])
{
    rassert(au != NULL);
    rassert(error_msg != NULL);

    error_msg[0] = '\0';

    const Au_streams* streams = Audio_unit_get_streams(au);
    if (streams == NULL)
        return true;

    Stream_target_dev_iter* iter =
        Stream_target_dev_iter_init(STREAM_TARGET_DEV_ITER_AUTO, streams);

    const char* name = Stream_target_dev_iter_get_next(iter);
    while (name != NULL)
    {
        const int proc_index = Au_streams_get_target_proc_index(streams, name);
        rassert(proc_index >= 0);

        const Processor* proc = Audio_unit_get_proc(au, proc_index);
        if ((proc == NULL) || !Device_is_existent((const Device*)proc))
        {
            sprintf(error_msg,
                    "Target of event stream interface `%.*s` not found in audio unit",
                    KQT_VAR_NAME_MAX,
                    name);
            return false;
        }

        const Device_impl* dimpl = ((const Device*)proc)->dimpl;
        if ((dimpl == NULL) || (dimpl->proc_type != Proc_type_stream))
        {
            sprintf(error_msg,
                    "Target of event stream interface `%.*s` is not a stream processor",
                    KQT_VAR_NAME_MAX,
                    name);
            return false;
        }

        name = Stream_target_dev_iter_get_next(iter);
    }

    return true;
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
    del_Au_event_map(au->event_map);
    Device_deinit(&au->parent);
    memory_free(au);

    return;
}


