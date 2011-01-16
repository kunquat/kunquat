

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <DSP_conf.h>
#include <DSP_table.h>
#include <Effect.h>
#include <Event_handler.h>
#include <Event_names.h>
#include <Event_type.h>
#include <File_base.h>
#include <Channel_state.h>
#include <General_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <Playdata.h>
#include <kunquat/limits.h>

#include <Event_control_pause.h>
#include <Event_control_resume.h>
#include <Event_control_play_pattern.h>

#include <Event_global_pattern_delay.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_subsong.h>
#include <Event_global_jump.h>

#include <Event_global_set_scale.h>
#include <Event_global_set_scale_offset.h>
#include <Event_global_mimic_scale.h>
#include <Event_global_shift_scale_intervals.h>

#include <Event_global_set_tempo.h>
#include <Event_global_set_volume.h>
#include <Event_global_slide_tempo.h>
#include <Event_global_slide_tempo_length.h>
#include <Event_global_slide_volume.h>
#include <Event_global_slide_volume_length.h>

#include <Event_channel_set_instrument.h>
#include <Event_channel_set_generator.h>
#include <Event_channel_set_effect.h>
#include <Event_channel_set_instrument_effects.h>
#include <Event_channel_set_dsp.h>
#include <Event_channel_set_dsp_context.h>

#include <Event_channel_note_on.h>
#include <Event_channel_note_off.h>

#include <Event_channel_set_force.h>
#include <Event_channel_slide_force.h>
#include <Event_channel_slide_force_length.h>
#include <Event_channel_tremolo_speed.h>
#include <Event_channel_tremolo_depth.h>
#include <Event_channel_tremolo_delay.h>

#include <Event_channel_slide_pitch.h>
#include <Event_channel_slide_pitch_length.h>
#include <Event_channel_vibrato_speed.h>
#include <Event_channel_vibrato_depth.h>
#include <Event_channel_vibrato_delay.h>
#include <Event_channel_arpeggio.h>

#include <Event_channel_set_lowpass.h>
#include <Event_channel_slide_lowpass.h>
#include <Event_channel_slide_lowpass_length.h>
#include <Event_channel_autowah_speed.h>
#include <Event_channel_autowah_depth.h>
#include <Event_channel_autowah_delay.h>

#include <Event_channel_set_resonance.h>

#include <Event_channel_set_panning.h>
#include <Event_channel_slide_panning.h>
#include <Event_channel_slide_panning_length.h>

#include <Event_channel_set_gen_bool.h>
#include <Event_channel_set_gen_int.h>
#include <Event_channel_set_gen_float.h>
#include <Event_channel_set_gen_reltime.h>

#include <Event_ins_set_pedal.h>

#include <Event_generator_set_bool.h>
#include <Event_generator_set_int.h>
#include <Event_generator_set_float.h>
#include <Event_generator_set_reltime.h>

#include <Event_effect_enable.h>
#include <Event_effect_disable.h>

#include <Event_dsp_set_bool.h>
#include <Event_dsp_set_int.h>
#include <Event_dsp_set_float.h>
#include <Event_dsp_set_reltime.h>

#include <xassert.h>
#include <xmemory.h>


struct Event_handler
{
    bool mute; // FIXME: this is just to make the stupid Channel_state_init happy
    Channel_state* ch_states[KQT_COLUMNS_MAX];
    Ins_table* insts;
    Effect_table* effects;
    DSP_table* dsps;
    Playdata* global_state;
    Event_names* event_names;
    bool (*control_process[EVENT_CONTROL_UPPER])(General_state*, char*);
    bool (*ch_process[EVENT_CHANNEL_UPPER])(Channel_state*, char*);
    bool (*global_process[EVENT_GLOBAL_UPPER])(Playdata*, char*);
    bool (*ins_process[EVENT_INS_UPPER])(Instrument_params*, char*);
    bool (*generator_process[EVENT_GENERATOR_UPPER])(Generator*, char*);
    bool (*effect_process[EVENT_EFFECT_UPPER])(Effect*, char*);
    bool (*dsp_process[EVENT_DSP_UPPER])(DSP_conf*, char*);
};


Event_handler* new_Event_handler(Playdata* global_state,
                                 Channel_state** ch_states,
                                 Ins_table* insts,
                                 Effect_table* effects,
                                 DSP_table* dsps)
{
    assert(global_state != NULL);
    assert(ch_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);
    assert(dsps != NULL);
    Event_handler* eh = xalloc(Event_handler);
    if (eh == NULL)
    {
        return NULL;
    }
    eh->event_names = new_Event_names();
    if (eh->event_names == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }
    eh->global_state = global_state;
/*    if (eh->global_state == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    } */
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        eh->ch_states[i] = ch_states[i];
//        Channel_state_init(&eh->ch_states[i], i, &eh->mute);
    }
    eh->insts = insts;
    eh->effects = effects;
    eh->dsps = dsps;

    Event_handler_set_control_process(eh, ">pause", EVENT_CONTROL_PAUSE,
                                      Event_control_pause_process);
    Event_handler_set_control_process(eh, ">resume", EVENT_CONTROL_RESUME,
                                      Event_control_resume_process);
    Event_handler_set_control_process(eh, ">pattern", EVENT_CONTROL_PLAY_PATTERN,
                                      Event_control_play_pattern_process);

    Event_handler_set_global_process(eh, "wpd", EVENT_GLOBAL_PATTERN_DELAY,
                                     Event_global_pattern_delay_process);
    Event_handler_set_global_process(eh, "w.jc", EVENT_GLOBAL_SET_JUMP_COUNTER,
                                     Event_global_set_jump_counter_process);
    Event_handler_set_global_process(eh, "w.jr", EVENT_GLOBAL_SET_JUMP_ROW,
                                     Event_global_set_jump_row_process);
    Event_handler_set_global_process(eh, "w.js", EVENT_GLOBAL_SET_JUMP_SECTION,
                                     Event_global_set_jump_section_process);
    Event_handler_set_global_process(eh, "w.jss", EVENT_GLOBAL_SET_JUMP_SUBSONG,
                                     Event_global_set_jump_subsong_process);
    Event_handler_set_global_process(eh, "wj", EVENT_GLOBAL_JUMP,
                                     Event_global_jump_process);

    Event_handler_set_global_process(eh, "w.s", EVENT_GLOBAL_SET_SCALE,
                                     Event_global_set_scale_process);
    Event_handler_set_global_process(eh, "w.so", EVENT_GLOBAL_SET_SCALE_OFFSET,
                                     Event_global_set_scale_offset_process);
    Event_handler_set_global_process(eh, "wms", EVENT_GLOBAL_MIMIC_SCALE,
                                     Event_global_mimic_scale_process);
    Event_handler_set_global_process(eh, "wssi", EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,
                                     Event_global_shift_scale_intervals_process);

    Event_handler_set_global_process(eh, "w.t", EVENT_GLOBAL_SET_TEMPO,
                                     Event_global_set_tempo_process);
    Event_handler_set_global_process(eh, "w.v", EVENT_GLOBAL_SET_VOLUME,
                                     Event_global_set_volume_process);
    Event_handler_set_global_process(eh, "w/t", EVENT_GLOBAL_SLIDE_TEMPO,
                                     Event_global_slide_tempo_process);
    Event_handler_set_global_process(eh, "w/=t", EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                                     Event_global_slide_tempo_length_process);
    Event_handler_set_global_process(eh, "w/v", EVENT_GLOBAL_SLIDE_VOLUME,
                                     Event_global_slide_volume_process);
    Event_handler_set_global_process(eh, "w/=v", EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                                     Event_global_slide_volume_length_process);

    Event_handler_set_ch_process(eh, "c.i", EVENT_CHANNEL_SET_INSTRUMENT,
                                 Event_channel_set_instrument_process);
    Event_handler_set_ch_process(eh, "c.g", EVENT_CHANNEL_SET_GENERATOR,
                                 Event_channel_set_generator_process);
    Event_handler_set_ch_process(eh, "c.e", EVENT_CHANNEL_SET_EFFECT,
                                 Event_channel_set_effect_process);
    Event_handler_set_ch_process(eh, "c.ie",
                                 EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS,
                                 Event_channel_set_instrument_effects_process);
    Event_handler_set_ch_process(eh, "c.d", EVENT_CHANNEL_SET_DSP,
                                 Event_channel_set_dsp_process);
    Event_handler_set_ch_process(eh, "c.dc", EVENT_CHANNEL_SET_DSP_CONTEXT,
                                 Event_channel_set_dsp_context_process);

    Event_handler_set_ch_process(eh, "cn+", EVENT_CHANNEL_NOTE_ON,
                                 Event_channel_note_on_process);
    Event_handler_set_ch_process(eh, "cn-", EVENT_CHANNEL_NOTE_OFF,
                                 Event_channel_note_off_process);

    Event_handler_set_ch_process(eh, "c.f", EVENT_CHANNEL_SET_FORCE,
                                 Event_channel_set_force_process);
    Event_handler_set_ch_process(eh, "c/f", EVENT_CHANNEL_SLIDE_FORCE,
                                 Event_channel_slide_force_process);
    Event_handler_set_ch_process(eh, "c/=f", EVENT_CHANNEL_SLIDE_FORCE_LENGTH,
                                 Event_channel_slide_force_length_process);
    Event_handler_set_ch_process(eh, "cTs", EVENT_CHANNEL_TREMOLO_SPEED,
                                 Event_channel_tremolo_speed_process);
    Event_handler_set_ch_process(eh, "cTd", EVENT_CHANNEL_TREMOLO_DEPTH,
                                 Event_channel_tremolo_depth_process);
    Event_handler_set_ch_process(eh, "cTdd", EVENT_CHANNEL_TREMOLO_DELAY,
                                 Event_channel_tremolo_delay_process);

    Event_handler_set_ch_process(eh, "c/p", EVENT_CHANNEL_SLIDE_PITCH,
                                 Event_channel_slide_pitch_process);
    Event_handler_set_ch_process(eh, "c/=p", EVENT_CHANNEL_SLIDE_PITCH_LENGTH,
                                 Event_channel_slide_pitch_length_process);
    Event_handler_set_ch_process(eh, "cVs", EVENT_CHANNEL_VIBRATO_SPEED,
                                 Event_channel_vibrato_speed_process);
    Event_handler_set_ch_process(eh, "cVd", EVENT_CHANNEL_VIBRATO_DEPTH,
                                 Event_channel_vibrato_depth_process);
    Event_handler_set_ch_process(eh, "cVdd", EVENT_CHANNEL_VIBRATO_DELAY,
                                 Event_channel_vibrato_delay_process);
    Event_handler_set_ch_process(eh, "cArp", EVENT_CHANNEL_ARPEGGIO,
                                 Event_channel_arpeggio_process);

    Event_handler_set_ch_process(eh, "c.l", EVENT_CHANNEL_SET_LOWPASS,
                                 Event_channel_set_lowpass_process);
    Event_handler_set_ch_process(eh, "c/l", EVENT_CHANNEL_SLIDE_LOWPASS,
                                 Event_channel_slide_lowpass_process);
    Event_handler_set_ch_process(eh, "c/=l", EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH,
                                 Event_channel_slide_lowpass_length_process);
    Event_handler_set_ch_process(eh, "cAs", EVENT_CHANNEL_AUTOWAH_SPEED,
                                 Event_channel_autowah_speed_process);
    Event_handler_set_ch_process(eh, "cAd", EVENT_CHANNEL_AUTOWAH_DEPTH,
                                 Event_channel_autowah_depth_process);
    Event_handler_set_ch_process(eh, "cAdd", EVENT_CHANNEL_AUTOWAH_DELAY,
                                 Event_channel_autowah_delay_process);

    Event_handler_set_ch_process(eh, "c.r", EVENT_CHANNEL_SET_RESONANCE,
                                 Event_channel_set_resonance_process);

    Event_handler_set_ch_process(eh, "c.P", EVENT_CHANNEL_SET_PANNING,
                                 Event_channel_set_panning_process);
    Event_handler_set_ch_process(eh, "c/P", EVENT_CHANNEL_SLIDE_PANNING,
                                 Event_channel_slide_panning_process);
    Event_handler_set_ch_process(eh, "c/=P", EVENT_CHANNEL_SLIDE_PANNING_LENGTH,
                                 Event_channel_slide_panning_length_process);

    Event_handler_set_ch_process(eh, "c.gB", EVENT_CHANNEL_SET_GEN_BOOL,
                                 Event_channel_set_gen_bool_process);
    Event_handler_set_ch_process(eh, "c.gI", EVENT_CHANNEL_SET_GEN_INT,
                                 Event_channel_set_gen_int_process);
    Event_handler_set_ch_process(eh, "c.gF", EVENT_CHANNEL_SET_GEN_FLOAT,
                                 Event_channel_set_gen_float_process);
    Event_handler_set_ch_process(eh, "c.gT", EVENT_CHANNEL_SET_GEN_RELTIME,
                                 Event_channel_set_gen_reltime_process);

    Event_handler_set_ins_process(eh, "i.ped", EVENT_INS_SET_PEDAL,
                                  Event_ins_set_pedal_process);

    Event_handler_set_generator_process(eh, "g.B", EVENT_GENERATOR_SET_BOOL,
                                        Event_generator_set_bool_process);
    Event_handler_set_generator_process(eh, "g.I", EVENT_GENERATOR_SET_INT,
                                        Event_generator_set_int_process);
    Event_handler_set_generator_process(eh, "g.F", EVENT_GENERATOR_SET_FLOAT,
                                        Event_generator_set_float_process);
    Event_handler_set_generator_process(eh, "g.T", EVENT_GENERATOR_SET_RELTIME,
                                        Event_generator_set_reltime_process);

    Event_handler_set_effect_process(eh, "e+", EVENT_EFFECT_ENABLE,
                                     Event_effect_enable_process);
    Event_handler_set_effect_process(eh, "e-", EVENT_EFFECT_DISABLE,
                                     Event_effect_disable_process);

    Event_handler_set_dsp_process(eh, "d.B", EVENT_DSP_SET_BOOL,
                                  Event_dsp_set_bool_process);
    Event_handler_set_dsp_process(eh, "d.I", EVENT_DSP_SET_INT,
                                  Event_dsp_set_int_process);
    Event_handler_set_dsp_process(eh, "d.F", EVENT_DSP_SET_FLOAT,
                                  Event_dsp_set_float_process);
    Event_handler_set_dsp_process(eh, "d.T", EVENT_DSP_SET_RELTIME,
                                  Event_dsp_set_reltime_process);

    if (Event_names_error(eh->event_names))
    {
        del_Event_handler(eh);
        return NULL;
    }
    return eh;
}


Event_names* Event_handler_get_names(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(Event_handler* eh,
                                  const char* name,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_CHANNEL(type));
    assert(ch_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->ch_process[type] = ch_process;
    return true;
}


bool Event_handler_set_control_process(Event_handler* eh,
                                       const char* name,
                                       Event_type type,
                                       bool (*control_process)(General_state*,
                                                               char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_CONTROL(type));
    assert(control_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->control_process[type] = control_process;
    return true;
}


bool Event_handler_set_global_process(Event_handler* eh,
                                      const char* name,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_GLOBAL(type));
    assert(global_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->global_process[type] = global_process;
    return true;
}


bool Event_handler_set_ins_process(Event_handler* eh,
                                   const char* name,
                                   Event_type type,
                                   bool (*ins_process)(Instrument_params*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_INS(type));
    assert(ins_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->ins_process[type] = ins_process;
    return true;
}


bool Event_handler_set_generator_process(Event_handler* eh,
                                         const char* name,
                                         Event_type type,
                                         bool (*gen_process)(Generator*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_GENERATOR(type));
    assert(gen_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->generator_process[type] = gen_process;
    return true;
}


bool Event_handler_set_effect_process(Event_handler* eh,
                                      const char* name,
                                      Event_type type,
                                      bool (*effect_process)(Effect*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_EFFECT(type));
    assert(effect_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->effect_process[type] = effect_process;
    return true;
}


bool Event_handler_set_dsp_process(Event_handler* eh,
                                   const char* name,
                                   Event_type type,
                                   bool (*dsp_process)(DSP_conf*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_DSP(type));
    assert(dsp_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->dsp_process[type] = dsp_process;
    return true;
}


bool Event_handler_handle(Event_handler* eh,
                          int index,
                          Event_type type,
                          char* fields)
{
    assert(eh != NULL);
    assert(EVENT_IS_VALID(type));
    if (EVENT_IS_CHANNEL(type))
    {
        assert(index >= 0);
        assert(index < KQT_COLUMNS_MAX);
        if (eh->ch_process[type] == NULL)
        {
            return false;
        }
        return eh->ch_process[type](eh->ch_states[index], fields);
    }
    else if (EVENT_IS_INS(type))
    {
        assert(index >= 0);
        assert(index < KQT_COLUMNS_MAX);
//        Instrument* ins = Ins_table_get(eh->insts, index);
        Instrument* ins = Ins_table_get(eh->insts,
                                        eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        return eh->ins_process[type](ins_params, fields);
    }
    else if (EVENT_IS_GLOBAL(type))
    {
        assert(index == -1);
        if (eh->global_process[type] == NULL)
        {
            return false;
        }
        return eh->global_process[type](eh->global_state, fields);
    }
    else if (EVENT_IS_GENERATOR(type))
    {
        assert(index >= 0);
        assert(index < KQT_COLUMNS_MAX);
        Instrument* ins = Ins_table_get(eh->insts,
                                        eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Generator* gen = Instrument_get_gen(ins,
                                            eh->ch_states[index]->generator);
        if (gen == NULL)
        {
            return false;
        }
        return eh->generator_process[type](gen, fields);
    }
    else if (EVENT_IS_EFFECT(type))
    {
        assert(index >= 0);
        assert(index < KQT_EFFECTS_MAX);
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(eh->insts,
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
        return eh->effect_process[type](eff, fields);
    }
    else if (EVENT_IS_DSP(type))
    {
        assert(index >= 0);
        assert(index < KQT_DSPS_MAX);
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(eh->insts,
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
#if 0
        if (eh->ch_states[index]->dsp_context >= 0)
        {
            Instrument* ins = Ins_table_get(eh->insts,
                                            eh->ch_states[index]->dsp_context);
            if (ins == NULL)
            {
                return false;
            }
            dsps = Instrument_get_dsps(ins);
        }
        if (dsps == NULL)
        {
            return false;
        }
#endif
        DSP_conf* conf = DSP_table_get_conf(dsps, eh->ch_states[index]->dsp);
        if (conf == NULL)
        {
            return false;
        }
        return eh->dsp_process[type](conf, fields);
    }
    else if (EVENT_IS_CONTROL(type))
    {
        return eh->control_process[type](&eh->global_state->parent, fields);
    }
    return false;
}


bool Event_handler_trigger(Event_handler* eh,
                           int index,
                           char* desc)
{
    assert(eh != NULL);
    assert(index >= -1);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    Read_state* state = READ_STATE_AUTO;
    desc = read_const_char(desc, '[', state);
    char event_name[EVENT_NAME_MAX + 2] = { '\0' };
    desc = read_string(desc, event_name, EVENT_NAME_MAX + 2, state);
    desc = read_const_char(desc, ',', state);
    if (state->error)
    {
        return false;
    }
    Event_type type = Event_names_get(eh->event_names, event_name);
    if (type == EVENT_NONE)
    {
        return false;
    }
    assert(Event_type_is_supported(type));
    if ((EVENT_IS_GLOBAL(type) != (index == -1)) &&
            !EVENT_IS_GENERAL(type) && !EVENT_IS_CONTROL(type))
    {
        return false;
    }
    return Event_handler_handle(eh, index, type, desc);
}


Playdata* Event_handler_get_global_state(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->global_state;
}


bool Event_handler_add_channel_gen_state_key(Event_handler* eh,
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
//    del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    xfree(eh);
    return;
}


