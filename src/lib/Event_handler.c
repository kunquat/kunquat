

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <DSP_conf.h>
#include <Effect.h>
#include <Event_handler.h>
#include <Event_names.h>
#include <Event_type.h>
#include <expr.h>
#include <File_base.h>
#include <General_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <string_common.h>
#include <Value.h>

#include <events/Event_control_decl.h>
#include <events/Event_general_decl.h>
#include <events/Event_master_decl.h>
#include <events/Event_channel_decl.h>
#include <events/Event_ins_decl.h>
#include <events/Event_generator_decl.h>
#include <events/Event_effect_decl.h>
#include <events/Event_dsp_decl.h>

#include <memory.h>
#include <xassert.h>


struct Event_handler
{
    Channel* channels[KQT_COLUMNS_MAX];
    Device_states* device_states;
    Master_params* master_params;
    Ins_table* insts;
    Effect_table* effects;
    Event_names* event_names;

    bool (*control_process[Event_control_STOP])(General_state*, Value*);
    bool (*general_process[Event_general_STOP])(General_state*, Value*);
    bool (*ch_process[Event_channel_STOP])(Channel*, Value*);
    bool (*master_process[Event_master_STOP])(Master_params*, Value*);
    bool (*ins_process[Event_ins_STOP])(
            Instrument_params*,
            Ins_state*,
            Value*);
    bool (*generator_process[Event_generator_STOP])(
            Generator*,
            Channel*,
            Value*);
    bool (*effect_process[Event_effect_STOP])(Effect*, Value*);
    bool (*dsp_process[Event_dsp_STOP])(DSP_conf*, Channel*, Value*);
};


Event_handler* new_Event_handler(
        Master_params* master_params,
        Channel** channels,
        Device_states* device_states,
        Ins_table* insts,
        Effect_table* effects)
{
    assert(master_params != NULL);
    assert(channels != NULL);
    assert(device_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);

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
    eh->insts = insts;
    eh->effects = effects;

#define EVENT_TYPE_DEF(type) Event_handler_set_control_process( \
        eh, Event_control_##type, Event_control_##type##_process);
#include <events/Event_control_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_general_process( \
        eh, Event_general_##type, Event_general_##type##_process);
#include <events/Event_general_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_master_process( \
        eh, Event_master_##type, Event_master_##type##_process);
#include <events/Event_master_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_ch_process( \
        eh, Event_channel_##type, Event_channel_##type##_process);
#include <events/Event_channel_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_ins_process( \
        eh, Event_ins_##type, Event_ins_##type##_process);
#include <events/Event_ins_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_generator_process( \
        eh, Event_generator_##type, Event_generator_##type##_process);
#include <events/Event_generator_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_effect_process( \
        eh, Event_effect_##type, Event_effect_##type##_process);
#include <events/Event_effect_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_dsp_process( \
        eh, Event_dsp_##type, Event_dsp_##type##_process);
#include <events/Event_dsp_types.h>

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
        bool (*ch_process)(Channel*, Value*))
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
        bool (*general_process)(General_state*, Value*))
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
        bool (*control_process)(General_state*, Value*))
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
        bool (*global_process)(Master_params*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_master(type));
    assert(global_process != NULL);

    eh->master_process[type] = global_process;

    return true;
}


bool Event_handler_set_ins_process(
        Event_handler* eh,
        Event_type type,
        bool (*ins_process)(Instrument_params*, Ins_state*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_ins(type));
    assert(ins_process != NULL);

    eh->ins_process[type] = ins_process;

    return true;
}


bool Event_handler_set_generator_process(
        Event_handler* eh,
        Event_type type,
        bool (*gen_process)(Generator*, Channel*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_generator(type));
    assert(gen_process != NULL);

    eh->generator_process[type] = gen_process;

    return true;
}


bool Event_handler_set_effect_process(
        Event_handler* eh,
        Event_type type,
        bool (*effect_process)(Effect*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_effect(type));
    assert(effect_process != NULL);

    eh->effect_process[type] = effect_process;

    return true;
}


bool Event_handler_set_dsp_process(
        Event_handler* eh,
        Event_type type,
        bool (*dsp_process)(DSP_conf*, Channel*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_dsp(type));
    assert(dsp_process != NULL);

    eh->dsp_process[type] = dsp_process;

    return true;
}


static bool Event_handler_handle(
        Event_handler* eh,
        int index,
        Event_type type,
        Value* value)
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

        return eh->ch_process[type](eh->channels[index], value);
    }
    else if (Event_is_ins(type))
    {
        Instrument* ins = Ins_table_get(
                eh->insts,
                eh->channels[index]->instrument);
        if (ins == NULL)
            return false;

        Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        Ins_state* ins_state = (Ins_state*)Device_states_get_state(
                eh->device_states,
                Device_get_id((Device*)ins));

        return eh->ins_process[type](ins_params, ins_state, value);
    }
    else if (Event_is_master(type))
    {
        if (eh->master_process[type] == NULL)
            return false;

        return eh->master_process[type](eh->master_params, value);
    }
    else if (Event_is_generator(type))
    {
        Instrument* ins = Ins_table_get(
                eh->insts,
                eh->channels[index]->instrument);
        if (ins == NULL)
            return false;

        Generator* gen = Instrument_get_gen(
                ins,
                eh->channels[index]->generator);
        if (gen == NULL)
            return false;

        return eh->generator_process[type](gen, eh->channels[index], value);
    }
    else if (Event_is_effect(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->channels[index]->inst_effects)
        {
            if (eh->channels[index]->effect >= KQT_INST_EFFECTS_MAX)
                return false;

            Instrument* ins = Ins_table_get(
                    eh->insts,
                    eh->channels[index]->instrument);
            if (ins == NULL)
                return false;

            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
            return false;

        Effect* eff = Effect_table_get(effects, eh->channels[index]->effect);
        if (eff == NULL)
            return false;

        return eh->effect_process[type](eff, value);
    }
    else if (Event_is_dsp(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->channels[index]->inst_effects)
        {
            if (eh->channels[index]->effect >= KQT_INST_EFFECTS_MAX)
                return false;

            Instrument* ins = Ins_table_get(
                    eh->insts,
                    eh->channels[index]->instrument);
            if (ins == NULL)
                return false;

            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
            return false;

        Effect* eff = Effect_table_get(effects, eh->channels[index]->effect);
        if (eff == NULL)
            return false;

        DSP_table* dsps = Effect_get_dsps(eff);
        DSP_conf* conf = DSP_table_get_conf(dsps, eh->channels[index]->dsp);
        if (conf == NULL)
            return false;

        return eh->dsp_process[type](conf, eh->channels[index], value);
    }
    else if (Event_is_control(type))
    {
        return eh->control_process[type](
                (General_state*)eh->master_params,
                value);
    }
    else if (Event_is_general(type))
    {
        General_state* gstate = (General_state*)eh->channels[index];
        return eh->general_process[type](gstate, value);
    }

    return false;
}


static void Event_handler_handle_query(
        Event_handler* eh,
        int index,
        Event_type event_type,
        Value* event_arg,
        bool silent)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(Event_is_query(event_type));
    assert(event_arg != NULL);
    (void)silent;

    //char auto_event[128] = "";
    Read_state* rs = READ_STATE_AUTO;
    switch (event_type)
    {
#if 0
        case Event_query_location:
        {
            if (eh->global_state->mode >= PLAY_SUBSONG)
            {
                snprintf(auto_event, 128, "[\"Atrack\", %" PRIu16 "]",
                         eh->global_state->track);
                Event_handler_trigger_const(eh, index, auto_event, silent, rs);
                snprintf(auto_event, 128, "[\"Asystem\", %" PRIu16 "]",
                         eh->global_state->system);
                Event_handler_trigger_const(eh, index, auto_event, silent, rs);
            }
            snprintf(auto_event, 128, "[\"Apattern\", %" PRId16 "]",
                     eh->global_state->piref.pat);
            Event_handler_trigger_const(eh, index, auto_event, silent, rs);
            snprintf(auto_event, 128,
                     "[\"Arow\", [%" PRId64 ", %" PRId32 "]]",
                     Tstamp_get_beats(&eh->global_state->pos),
                     Tstamp_get_rem(&eh->global_state->pos));
            Event_handler_trigger_const(eh, index, auto_event, silent, rs);
        } break;
        case Event_query_voice_count:
        {
            snprintf(auto_event, 128, "[\"Avoices\", %d]",
                     eh->global_state->active_voices);
            eh->global_state->active_voices = 0;
            Event_handler_trigger_const(eh, index, auto_event, silent, rs);
        } break;
        case Event_query_actual_force:
        {
            assert(event_arg->type == VALUE_TYPE_INT);
            assert(event_arg->value.int_type >= 0);
            assert(event_arg->value.int_type < KQT_GENERATORS_MAX);
            double force = Channel_state_get_fg_force(eh->ch_states[index],
                                                event_arg->value.int_type);
            if (!isnan(force))
            {
                snprintf(auto_event, 128, "[\"Af\", %f]", force);
                Event_handler_trigger_const(eh, index, auto_event, silent, rs);
            }
        } break;
#endif
        default:
            assert(false);
    }

    assert(!rs->error);
    return;
}


static bool Event_handler_act(
        Event_handler* eh,
        bool silent,
        int index,
        char* event_name,
        Event_type event_type,
        Value* value)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(event_name != NULL);
    assert(value != NULL);

    assert(eh->channels[index]->freq != NULL);
    assert(*eh->channels[index]->freq > 0);
    assert(eh->channels[index]->tempo != NULL);
    assert(*eh->channels[index]->tempo > 0);

    if (!Event_is_query(event_type) && !Event_is_auto(event_type) &&
            !Event_handler_handle(eh, index, event_type, value))
        return false;

    if (Event_is_query(event_type))
        Event_handler_handle_query(eh, index, event_type, value, silent);

#if 0
    Target_event* bound = Bind_get_first(
            eh->global_state->bind,
            eh->channels[index]->event_cache,
            eh->global_state->parent.env,
            event_name,
            value,
            eh->channels[index]->rand);
    while (bound != NULL)
    {
        Event_handler_trigger(
                eh,
                (index + bound->ch_offset + KQT_COLUMNS_MAX) % KQT_COLUMNS_MAX,
                bound->desc,
                silent,
                value);
        bound = bound->next;
    }
#endif
    return true;
}


bool Event_handler_trigger(
        Event_handler* eh,
        int ch_num,
        char* name,
        Value* arg)
{
    assert(eh != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(name != NULL);
    assert(arg != NULL);

    Event_type type = Event_names_get(eh->event_names, name);
    assert(type != Event_NONE);

    return Event_handler_act(
            eh,
            false, // silent
            ch_num,
            name,
            type,
            arg);
}


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


bool Event_handler_add_channel_gen_state_key(
        Event_handler* eh,
        const char* key)
{
    assert(eh != NULL);
    assert(key != NULL);

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Channel_gen_state_set_key(eh->channels[i]->cgstate, key))
            return false;
    }

    return true;
}


void del_Event_handler(Event_handler* eh)
{
    if (eh == NULL)
        return;

    del_Event_names(eh->event_names);
    memory_free(eh);

    return;
}


