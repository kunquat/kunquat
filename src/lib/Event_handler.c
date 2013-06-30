

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
#include <Event_buffer.h>
#include <Event_handler.h>
#include <Event_names.h>
#include <Event_type.h>
#include <expr.h>
#include <File_base.h>
#include <Channel_state.h>
#include <General_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <Playdata.h>
#include <kunquat/limits.h>
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
    Channel_state* ch_states[KQT_COLUMNS_MAX];
    Ins_table* insts;
    Effect_table* effects;
    Master_params* master_params;
    Playdata* global_state;
    Event_names* event_names;
    Event_buffer* event_buffer;
    Event_buffer* tracker_buffer;
    bool (*control_process[Event_control_STOP])(General_state*, General_state*, Value*);
    bool (*general_process[Event_general_STOP])(General_state*, Value*);
    bool (*ch_process[Event_channel_STOP])(Channel_state*, Value*);
    bool (*global_process[Event_master_STOP])(Master_params*, Playdata*, Value*);
    bool (*ins_process[Event_ins_STOP])(Instrument_params*, Value*);
    bool (*generator_process[Event_generator_STOP])(
            Generator*,
            Channel_state*,
            Value*);
    bool (*effect_process[Event_effect_STOP])(Effect*, Value*);
    bool (*dsp_process[Event_dsp_STOP])(DSP_conf*, Channel_state*, Value*);
};


Event_handler* new_Event_handler(
        Master_params* master_params,
        Playdata* global_state,
        Channel_state** ch_states,
        Ins_table* insts,
        Effect_table* effects)
{
    assert(master_params != NULL || global_state != NULL);
    //assert(master_params != NULL);
    //assert(global_state != NULL);
    assert(ch_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);

    Event_handler* eh = memory_alloc_item(Event_handler);
    if (eh == NULL)
    {
        return NULL;
    }

    eh->event_buffer = new_Event_buffer(16384);
    eh->tracker_buffer = new_Event_buffer(16384);
    eh->event_names = new_Event_names();
    if (eh->event_buffer == NULL ||
            eh->tracker_buffer == NULL ||
            eh->event_names == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }

    eh->master_params = master_params;
    eh->global_state = global_state;
/*    if (eh->global_state == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    } */
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        eh->ch_states[i] = ch_states[i];
    }
    eh->insts = insts;
    eh->effects = effects;

#define EVENT_TYPE_DEF(type) Event_handler_set_control_process( \
        eh, Event_control_##type, Event_control_##type##_process);
#include <events/Event_control_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_general_process( \
        eh, Event_general_##type, Event_general_##type##_process);
#include <events/Event_general_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_master_process( \
        eh, Event_master_##type, Event_global_##type##_process);
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

    if (eh->master_params != NULL)
    {
    }
    else
    {
        Playdata_set_event_filter(global_state, eh->event_names);
    }
    return eh;
}


Event_names* Event_handler_get_names(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(
        Event_handler* eh,
        Event_type type,
        bool (*ch_process)(Channel_state*, Value*))
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
        bool (*control_process)(General_state*, General_state*, Value*))
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
        bool (*global_process)(Master_params*, Playdata*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_master(type));
    assert(global_process != NULL);
    eh->global_process[type] = global_process;
    return true;
}


bool Event_handler_set_ins_process(
        Event_handler* eh,
        Event_type type,
        bool (*ins_process)(Instrument_params*, Value*))
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
        bool (*gen_process)(Generator*, Channel_state*, Value*))
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
        bool (*dsp_process)(DSP_conf*, Channel_state*, Value*))
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
    assert(eh->ch_states[index]->freq != NULL);
    assert(*eh->ch_states[index]->freq > 0);
    assert(eh->ch_states[index]->tempo != NULL);
    assert(*eh->ch_states[index]->tempo > 0);
    if (Event_is_channel(type))
    {
        if (eh->ch_process[type] == NULL)
        {
            return false;
        }
        return eh->ch_process[type](eh->ch_states[index], value);
    }
    else if (Event_is_ins(type))
    {
        Instrument* ins = Ins_table_get(
                eh->insts,
                eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        return eh->ins_process[type](ins_params, value);
    }
    else if (Event_is_master(type))
    {
        if (eh->global_process[type] == NULL)
        {
            return false;
        }
        return eh->global_process[type](eh->master_params, eh->global_state, value);
    }
    else if (Event_is_generator(type))
    {
        Instrument* ins = Ins_table_get(
                eh->insts,
                eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Generator* gen = Instrument_get_gen(
                ins,
                eh->ch_states[index]->generator);
        if (gen == NULL)
        {
            return false;
        }
        return eh->generator_process[type](gen, eh->ch_states[index], value);
    }
    else if (Event_is_effect(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(
                    eh->insts,
                    eh->ch_states[index]->instrument);
            if (ins == NULL)
            {
                return false;
            }
            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
        {
            return false;
        }
        Effect* eff = Effect_table_get(effects, eh->ch_states[index]->effect);
        if (eff == NULL)
        {
            return false;
        }
        return eh->effect_process[type](eff, value);
    }
    else if (Event_is_dsp(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(
                    eh->insts,
                    eh->ch_states[index]->instrument);
            if (ins == NULL)
            {
                return false;
            }
            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
        {
            return false;
        }
        Effect* eff = Effect_table_get(effects, eh->ch_states[index]->effect);
        if (eff == NULL)
        {
            return false;
        }
        DSP_table* dsps = Effect_get_dsps(eff);
        DSP_conf* conf = DSP_table_get_conf(dsps, eh->ch_states[index]->dsp);
        if (conf == NULL)
        {
            return false;
        }
        return eh->dsp_process[type](conf, eh->ch_states[index], value);
    }
    else if (Event_is_control(type))
    {
        return eh->control_process[type](
                (General_state*)eh->master_params,
                (General_state*)eh->global_state,
                value);
    }
    else if (Event_is_general(type))
    {
        General_state* gstate = (General_state*)eh->global_state;
        if (index >= 0)
        {
            gstate = (General_state*)eh->ch_states[index];
        }
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

    char auto_event[128] = "";
    Read_state* rs = READ_STATE_AUTO;
    switch (event_type)
    {
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

    assert(eh->ch_states[index]->freq != NULL);
    assert(*eh->ch_states[index]->freq > 0);
    assert(eh->ch_states[index]->tempo != NULL);
    assert(*eh->ch_states[index]->tempo > 0);

    if (!Event_is_query(event_type) && !Event_is_auto(event_type) &&
            !Event_handler_handle(eh, index, event_type, value))
    {
        return false;
    }
    if (!silent)
    {
        if (Event_names_get_pass(eh->event_names, event_name))
        {
            Event_buffer_add(eh->event_buffer, index, event_name, value);
        }
        //if ((event_type >= Event_control_env_set_bool_name &&
        //        event_type <= Event_control_env_set_tstamp) ||
        //    Event_is_auto(event_type))
        {
            Event_buffer_add(eh->tracker_buffer, index, event_name, value);
        }
    }
    if (Event_is_query(event_type))
    {
        Event_handler_handle_query(eh, index, event_type, value, silent);
    }

    if (eh->global_state != NULL)
    {
        Target_event* bound = Bind_get_first(
                eh->global_state->bind,
                eh->ch_states[index]->event_cache,
                eh->global_state->parent.env,
                event_name,
                value,
                eh->ch_states[index]->rand);
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
    }
    return true;
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
    {
        return false;
    }
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
    if (!General_state_events_enabled((General_state*)eh->ch_states[index]) &&
            *event_type != Event_general_if &&
            *event_type != Event_general_else &&
            *event_type != Event_general_end_if)
    {
        return false;
    }
    return true;
}


bool Event_handler_trigger(
        Event_handler* eh,
        int index,
        char* desc,
        bool silent,
        Value* meta)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);

    assert(eh->ch_states[index]->freq != NULL);
    assert(*eh->ch_states[index]->freq > 0);
    assert(eh->ch_states[index]->tempo != NULL);
    assert(*eh->ch_states[index]->tempo > 0);

    Read_state* state = READ_STATE_AUTO;
    char event_name[EVENT_NAME_MAX + 2] = "";
    Event_type event_type = Event_NONE;
    if (!Event_handler_process_type(
                eh,
                index,
                &desc,
                event_name,
                &event_type,
                state))
    {
        return !state->error;
    }
    assert(!state->error);
    Value* value = VALUE_AUTO;
    Value_type field_type = Event_names_get_param_type(
            eh->event_names,
            event_name);
    if (field_type == VALUE_TYPE_NONE)
    {
        value->type = VALUE_TYPE_NONE;
        desc = read_null(desc, state);
    }
    else
    {
        char* quote_pos = strrchr(event_name, '"');
        if (quote_pos != NULL && string_eq(quote_pos, "\""))
        {
            value->type = VALUE_TYPE_STRING;
            desc = read_string(desc, value->value.string_type,
                               ENV_VAR_NAME_MAX, state);
        }
        else
        {
            Environment* env = NULL;
            if (eh->master_params != NULL)
                env = eh->master_params->parent.env;
            else
                env = eh->global_state->parent.env;

            desc = evaluate_expr(
                    desc,
                    env,
                    state,
                    meta,
                    value,
                    eh->ch_states[index]->rand);
            desc = read_const_char(desc, '"', state);
        }
        switch (field_type)
        {
            case VALUE_TYPE_BOOL:
            {
                if (value->type != VALUE_TYPE_BOOL)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case VALUE_TYPE_INT:
            {
                if (value->type == VALUE_TYPE_FLOAT)
                {
                    value->type = VALUE_TYPE_INT;
                    value->value.int_type = value->value.float_type;
                }
                else if (value->type != VALUE_TYPE_INT)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case VALUE_TYPE_FLOAT:
            {
                if (value->type == VALUE_TYPE_INT)
                {
                    value->type = VALUE_TYPE_FLOAT;
                    value->value.float_type = value->value.int_type;
                }
                else if (value->type != VALUE_TYPE_FLOAT)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case VALUE_TYPE_REAL:
            {
                assert(false);
            } break;
            case VALUE_TYPE_TSTAMP:
            {
                if (value->type == VALUE_TYPE_INT)
                {
                    value->type = VALUE_TYPE_TSTAMP;
                    Tstamp_set(&value->value.Tstamp_type,
                                value->value.int_type, 0);
                }
                else if (value->type == VALUE_TYPE_FLOAT)
                {
                    value->type = VALUE_TYPE_TSTAMP;
                    double beats = floor(value->value.float_type);
                    Tstamp_set(&value->value.Tstamp_type, beats,
                                (value->value.float_type - beats) *
                                    KQT_TSTAMP_BEAT);
                }
                else if (value->type != VALUE_TYPE_TSTAMP)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case VALUE_TYPE_STRING:
            {
                if (value->type != VALUE_TYPE_STRING)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case VALUE_TYPE_PAT_INST_REF:
            {
                if (value->type != VALUE_TYPE_PAT_INST_REF)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            default:
                assert(false);
        }
    }
    desc = read_const_char(desc, ']', state);
    if (state->error)
    {
        return false;
    }
    return Event_handler_act(
            eh,
            silent,
            index,
            event_name,
            event_type,
            value);
}


bool Event_handler_trigger_const(
        Event_handler* eh,
        int index,
        char* desc,
        bool silent,
        Read_state* rs)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    assert(rs != NULL);

    char event_name[EVENT_NAME_MAX + 2] = "";
    Event_type event_type = Event_NONE;
    if (!Event_handler_process_type(
                eh,
                index,
                &desc,
                event_name,
                &event_type,
                rs))
    {
        return !rs->error;
    }
    assert(!rs->error);
    Value* value = VALUE_AUTO;
    Value_type field_type = Event_names_get_param_type(
            eh->event_names,
            event_name);
    switch (field_type)
    {
        case VALUE_TYPE_NONE:
        {
            value->type = VALUE_TYPE_NONE;
            desc = read_null(desc, rs);
        } break;
        case VALUE_TYPE_BOOL:
        {
            value->type = VALUE_TYPE_BOOL;
            desc = read_bool(desc, &value->value.bool_type, rs);
        } break;
        case VALUE_TYPE_INT:
        {
            value->type = VALUE_TYPE_INT;
            desc = read_int(desc, &value->value.int_type, rs);
        } break;
        case VALUE_TYPE_FLOAT:
        {
            value->type = VALUE_TYPE_FLOAT;
            desc = read_double(desc, &value->value.float_type, rs);
        } break;
        case VALUE_TYPE_REAL:
        {
            assert(false);
        } break;
        case VALUE_TYPE_TSTAMP:
        {
            value->type = VALUE_TYPE_TSTAMP;
            desc = read_tstamp(desc, &value->value.Tstamp_type, rs);
        } break;
        case VALUE_TYPE_STRING:
        {
            value->type = VALUE_TYPE_STRING;
            desc = read_string(
                    desc,
                    value->value.string_type,
                    ENV_VAR_NAME_MAX,
                    rs);
        } break;
        case VALUE_TYPE_PAT_INST_REF:
        {
            value->type = VALUE_TYPE_PAT_INST_REF;
            desc = read_pat_inst_ref(
                    desc,
                    &value->value.Pat_inst_ref_type,
                    rs);
        } break;
        default:
            assert(false);
    }
    desc = read_const_char(desc, ']', rs);
    if (rs->error)
    {
        return false;
    }
    return Event_handler_act(
            eh,
            silent,
            index,
            event_name,
            event_type,
            value);
}


bool Event_handler_receive(Event_handler* eh, char* dest, int size)
{
    assert(eh != NULL);
    assert(dest != NULL);
    assert(size > 0);
    return Event_buffer_get(eh->event_buffer, dest, size);
}


bool Event_handler_treceive(Event_handler* eh, char* dest, int size)
{
    assert(eh != NULL);
    assert(dest != NULL);
    assert(size > 0);
    return Event_buffer_get(eh->tracker_buffer, dest, size);
}


Playdata* Event_handler_get_global_state(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->global_state;
}


void Event_handler_clear_buffers(Event_handler* eh)
{
    assert(eh != NULL);
    Event_buffer_clear(eh->event_buffer);
    Event_buffer_clear(eh->tracker_buffer);
    return;
}


bool Event_handler_add_channel_gen_state_key(
        Event_handler* eh,
        const char* key)
{
    assert(eh != NULL);
    assert(key != NULL);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Channel_gen_state_set_key(eh->ch_states[i]->cgstate, key))
        {
            return false;
        }
    }
    return true;
}


void del_Event_handler(Event_handler* eh)
{
    if (eh == NULL)
    {
        return;
    }
    del_Event_names(eh->event_names);
    del_Event_buffer(eh->event_buffer);
    del_Event_buffer(eh->tracker_buffer);
//    del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    memory_free(eh);
    return;
}


