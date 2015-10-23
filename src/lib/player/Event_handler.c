

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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <debug/assert.h>
#include <devices/Audio_unit.h>
#include <devices/Processor.h>
#include <kunquat/limits.h>
#include <module/Au_table.h>
#include <module/Bind.h>
#include <module/Module.h>
#include <player/Channel.h>
#include <player/Event_handler.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <player/General_state.h>
#include <string/Streader.h>
#include <string/common.h>
#include <Value.h>

#include <player/events/Event_control_decl.h>
#include <player/events/Event_general_decl.h>
#include <player/events/Event_master_decl.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_au_decl.h>

#include <memory.h>


struct Event_handler
{
    Channel* channels[KQT_COLUMNS_MAX];
    Device_states* device_states;
    Master_params* master_params;
    Au_table* au_table;
    Event_names* event_names;

    bool (*control_process[Event_control_STOP])(General_state*, Channel*, const Value*);
    bool (*general_process[Event_general_STOP])(General_state*, const Value*);
    bool (*ch_process[Event_channel_STOP])(Channel*, Device_states*, const Value*);
    bool (*master_process[Event_master_STOP])(Master_params*, const Value*);
    bool (*au_process[Event_au_STOP])(
            const Audio_unit*,
            const Au_params*,
            Au_state*,
            Channel*,
            Device_states*,
            const Value*);
};


Event_handler* new_Event_handler(
        Master_params* master_params,
        Channel** channels,
        Device_states* device_states,
        Au_table* au_table)
{
    assert(master_params != NULL);
    assert(channels != NULL);
    assert(device_states != NULL);
    assert(au_table != NULL);

    Event_handler* eh = memory_alloc_item(Event_handler);
    if (eh == NULL)
        return NULL;

    eh->event_names = new_Event_names();
    if (eh->event_names == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }

    eh->master_params = master_params;
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        eh->channels[i] = channels[i];
    eh->device_states = device_states;
    eh->au_table = au_table;

#define EVENT_CONTROL_DEF(name, type_suffix, arg_type, validator)                \
        Event_handler_set_control_process(                                       \
        eh, Event_control_##type_suffix, Event_control_##type_suffix##_process);
#include <player/events/Event_control_types.h>

#define EVENT_GENERAL_DEF(name, type_suffix, arg_type, validator)                \
        Event_handler_set_general_process(                                       \
        eh, Event_general_##type_suffix, Event_general_##type_suffix##_process);
#include <player/events/Event_general_types.h>

#define EVENT_MASTER_DEF(name, type_suffix, arg_type, validator)               \
        Event_handler_set_master_process(                                      \
        eh, Event_master_##type_suffix, Event_master_##type_suffix##_process);
#include <player/events/Event_master_types.h>

#define EVENT_CHANNEL_DEF(name, type_suffix, arg_type, validator)                \
        Event_handler_set_ch_process(                                            \
        eh, Event_channel_##type_suffix, Event_channel_##type_suffix##_process);
#include <player/events/Event_channel_types.h>

#define EVENT_AU_DEF(name, type_suffix, arg_type, validator)           \
        Event_handler_set_au_process(                                  \
        eh, Event_au_##type_suffix, Event_au_##type_suffix##_process);
#include <player/events/Event_au_types.h>

    return eh;
}


const Event_names* Event_handler_get_names(const Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(
        Event_handler* eh,
        Event_type type,
        bool (*ch_process)(Channel*, Device_states*, const Value*))
{
    assert(eh != NULL);
    assert(Event_is_channel(type));
    assert(ch_process != NULL);

    eh->ch_process[type] = ch_process;

    return true;
}


bool Event_handler_set_general_process(
        Event_handler* eh,
        Event_type type,
        bool (*general_process)(General_state*, const Value*))
{
    assert(eh != NULL);
    assert(Event_is_general(type));
    assert(general_process != NULL);

    eh->general_process[type] = general_process;

    return true;
}


bool Event_handler_set_control_process(
        Event_handler* eh,
        Event_type type,
        bool (*control_process)(General_state*, Channel*, const Value*))
{
    assert(eh != NULL);
    assert(Event_is_control(type));
    assert(control_process != NULL);

    eh->control_process[type] = control_process;

    return true;
}


bool Event_handler_set_master_process(
        Event_handler* eh,
        Event_type type,
        bool (*global_process)(Master_params*, const Value*))
{
    assert(eh != NULL);
    assert(Event_is_master(type));
    assert(global_process != NULL);

    eh->master_process[type] = global_process;

    return true;
}


bool Event_handler_set_au_process(
        Event_handler* eh,
        Event_type type,
        bool (*au_process)(
            const Audio_unit*,
            const Au_params*,
            Au_state*,
            Channel*,
            Device_states*,
            const Value*))
{
    assert(eh != NULL);
    assert(Event_is_au(type));
    assert(au_process != NULL);

    eh->au_process[type] = au_process;

    return true;
}


static bool Event_handler_handle(
        Event_handler* eh,
        int index,
        Event_type type,
        const Value* value)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(Event_is_valid(type));
    assert(eh->channels[index]->freq != NULL);
    assert(*eh->channels[index]->freq > 0);
    assert(eh->channels[index]->tempo != NULL);
    assert(*eh->channels[index]->tempo > 0);

    if (Event_is_channel(type))
    {
        if (eh->ch_process[type] == NULL)
            return false;

        return eh->ch_process[type](
                eh->channels[index],
                eh->device_states,
                value);
    }
    else if (Event_is_au(type))
    {
        // Find our audio unit
        Audio_unit* au = Module_get_au_from_input(
                eh->master_params->parent.module,
                eh->channels[index]->au_input);
        if (au == NULL)
            return false;

        const Au_params* au_params = Audio_unit_get_params(au);
        assert(au_params != NULL);
        Au_state* au_state = (Au_state*)Device_states_get_state(
                eh->device_states,
                Device_get_id((Device*)au));

        return eh->au_process[type](
                au, au_params, au_state, eh->channels[index], eh->device_states, value);
    }
    else if (Event_is_master(type))
    {
        if (eh->master_process[type] == NULL)
            return false;

        return eh->master_process[type](eh->master_params, value);
    }
    else if (Event_is_control(type))
    {
        return eh->control_process[type](
                (General_state*)eh->master_params, eh->channels[index], value);
    }
    else if (Event_is_general(type))
    {
        General_state* gstate = (General_state*)eh->channels[index];
        return eh->general_process[type](gstate, value);
    }

    return false;
}


bool Event_handler_trigger(
        Event_handler* eh,
        int ch_num,
        const char* name,
        const Value* arg)
{
    assert(eh != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(name != NULL);
    assert(arg != NULL);

    Event_type type = Event_names_get(eh->event_names, name);
    assert(type != Event_NONE);
    assert(!Event_is_query(type));
    assert(!Event_is_auto(type));

    assert(eh->channels[ch_num]->freq != NULL);
    assert(*eh->channels[ch_num]->freq > 0);
    assert(eh->channels[ch_num]->tempo != NULL);
    assert(*eh->channels[ch_num]->tempo > 0);

    return Event_handler_handle(eh, ch_num, type, arg);
}


#if 0
bool Event_handler_process_type(
        Event_handler* eh,
        int index,
        char** desc,
        char* event_name,
        Event_type* event_type,
        Read_state* state)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    assert(*desc != NULL);
    assert(event_name != NULL);
    assert(event_type != NULL);
    assert(state != NULL);
    assert(!state->error);

    *desc = read_const_char(*desc, '[', state);
    *desc = read_string(*desc, event_name, EVENT_NAME_MAX + 2, state);
    *desc = read_const_char(*desc, ',', state);
    if (state->error)
        return false;

    *event_type = Event_names_get(eh->event_names, event_name);
    if (*event_type == Event_NONE)
    {
        Read_state_set_error(
                state,
                "Unsupported event type: %s",
                event_name);
        return false;
    }

    assert(Event_is_valid(*event_type));
    if (!General_state_events_enabled((General_state*)eh->channels[index]) &&
            *event_type != Event_general_if &&
            *event_type != Event_general_else &&
            *event_type != Event_general_end_if)
        return false;

    return true;
}
#endif


#if 0
bool Event_handler_add_channel_proc_state_key(
        Event_handler* eh,
        const char* key)
{
    assert(eh != NULL);
    assert(key != NULL);

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Channel_proc_state_set_key(eh->channels[i]->cgstate, key))
            return false;
    }

    return true;
}
#endif


void del_Event_handler(Event_handler* eh)
{
    if (eh == NULL)
        return;

    del_Event_names(eh->event_names);
    memory_free(eh);

    return;
}


