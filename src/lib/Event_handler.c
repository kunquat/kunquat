

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <DSP_conf.h>
#include <DSP_table.h>
#include <Event_handler.h>
#include <Event_type.h>
#include <Channel_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <Playdata.h>
#include <kunquat/limits.h>

#include <Event_global_pattern_delay.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_subsong.h>

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

#include <Event_channel_set_filter.h>
#include <Event_channel_slide_filter.h>
#include <Event_channel_slide_filter_length.h>
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

#include <Event_dsp_set_bool.h>
#include <Event_dsp_set_float.h>

#include <xassert.h>
#include <xmemory.h>


struct Event_handler
{
    bool mute; // FIXME: this is just to make the stupid Channel_state_init happy
    Channel_state* ch_states[KQT_COLUMNS_MAX];
    Ins_table* insts;
    DSP_table* dsps;
    Playdata* global_state;
    bool (*ch_process[EVENT_CHANNEL_UPPER])(Channel_state*, char*);
    bool (*global_process[EVENT_GLOBAL_UPPER])(Playdata*, char*);
    bool (*ins_process[EVENT_INS_UPPER])(Instrument_params*, char*);
    bool (*generator_process[EVENT_GENERATOR_UPPER])(Generator*, char*);
    bool (*dsp_process[EVENT_DSP_UPPER])(DSP_conf*, char*);
};


Event_handler* new_Event_handler(Playdata* global_state,
                                 Channel_state** ch_states,
                                 Ins_table* insts,
                                 DSP_table* dsps)
{
    assert(global_state != NULL);
    assert(ch_states != NULL);
    assert(insts != NULL);
    assert(dsps != NULL);
    Event_handler* eh = xalloc(Event_handler);
    if (eh == NULL)
    {
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
    eh->dsps = dsps;

    Event_handler_set_global_process(eh, EVENT_GLOBAL_PATTERN_DELAY,
                                     Event_global_pattern_delay_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_JUMP_COUNTER,
                                     Event_global_set_jump_counter_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_JUMP_ROW,
                                     Event_global_set_jump_row_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_JUMP_SECTION,
                                     Event_global_set_jump_section_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_JUMP_SUBSONG,
                                     Event_global_set_jump_subsong_process);

    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_SCALE,
                                     Event_global_set_scale_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_SCALE_OFFSET,
                                     Event_global_set_scale_offset_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_MIMIC_SCALE,
                                     Event_global_mimic_scale_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,
                                     Event_global_shift_scale_intervals_process);

    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_TEMPO,
                                     Event_global_set_tempo_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_VOLUME,
                                     Event_global_set_volume_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SLIDE_TEMPO,
                                     Event_global_slide_tempo_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                                     Event_global_slide_tempo_length_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SLIDE_VOLUME,
                                     Event_global_slide_volume_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                                     Event_global_slide_volume_length_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_INSTRUMENT,
                                 Event_channel_set_instrument_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GENERATOR,
                                 Event_channel_set_generator_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_NOTE_ON,
                                 Event_channel_note_on_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_NOTE_OFF,
                                 Event_channel_note_off_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_FORCE,
                                 Event_channel_set_force_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_FORCE,
                                 Event_channel_slide_force_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_FORCE_LENGTH,
                                 Event_channel_slide_force_length_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_TREMOLO_SPEED,
                                 Event_channel_tremolo_speed_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_TREMOLO_DEPTH,
                                 Event_channel_tremolo_depth_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_TREMOLO_DELAY,
                                 Event_channel_tremolo_delay_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_PITCH,
                                 Event_channel_slide_pitch_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_PITCH_LENGTH,
                                 Event_channel_slide_pitch_length_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_VIBRATO_SPEED,
                                 Event_channel_vibrato_speed_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_VIBRATO_DEPTH,
                                 Event_channel_vibrato_depth_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_VIBRATO_DELAY,
                                 Event_channel_vibrato_delay_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_ARPEGGIO,
                                 Event_channel_arpeggio_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_FILTER,
                                 Event_channel_set_filter_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_FILTER,
                                 Event_channel_slide_filter_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_FILTER_LENGTH,
                                 Event_channel_slide_filter_length_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_AUTOWAH_SPEED,
                                 Event_channel_autowah_speed_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_AUTOWAH_DEPTH,
                                 Event_channel_autowah_depth_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_AUTOWAH_DELAY,
                                 Event_channel_autowah_delay_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_RESONANCE,
                                 Event_channel_set_resonance_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_PANNING,
                                 Event_channel_set_panning_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_PANNING,
                                 Event_channel_slide_panning_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_PANNING_LENGTH,
                                 Event_channel_slide_panning_length_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_BOOL,
                                 Event_channel_set_gen_bool_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_INT,
                                 Event_channel_set_gen_int_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_FLOAT,
                                 Event_channel_set_gen_float_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_RELTIME,
                                 Event_channel_set_gen_reltime_process);

    Event_handler_set_ins_process(eh, EVENT_INS_SET_PEDAL,
                                  Event_ins_set_pedal_process);

    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_BOOL,
                                        Event_generator_set_bool_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_INT,
                                        Event_generator_set_int_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_FLOAT,
                                        Event_generator_set_float_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_RELTIME,
                                        Event_generator_set_reltime_process);

    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_BOOL,
                                  Event_dsp_set_bool_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_FLOAT,
                                  Event_dsp_set_float_process);

    return eh;
}


void Event_handler_set_ch_process(Event_handler* eh,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_CHANNEL(type));
    assert(ch_process != NULL);
    eh->ch_process[type] = ch_process;
    return;
}


void Event_handler_set_global_process(Event_handler* eh,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_GLOBAL(type));
    assert(global_process != NULL);
    eh->global_process[type] = global_process;
    return;
}


void Event_handler_set_ins_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*ins_process)(Instrument_params*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_INS(type));
    assert(ins_process != NULL);
    eh->ins_process[type] = ins_process;
    return;
}


void Event_handler_set_generator_process(Event_handler* eh,
                                         Event_type type,
                                         bool (*gen_process)(Generator*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_GENERATOR(type));
    assert(gen_process != NULL);
    eh->generator_process[type] = gen_process;
    return;
}


void Event_handler_set_dsp_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*dsp_process)(DSP_conf*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_DSP(type));
    assert(dsp_process != NULL);
    eh->dsp_process[type] = dsp_process;
    return;
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
        assert(index < KQT_INSTRUMENTS_MAX);
        Instrument* ins = Ins_table_get(eh->insts, index);
        if (ins != NULL)
        {
            Instrument_params* ins_params = Instrument_get_params(ins);
            assert(ins_params != NULL);
            return eh->ins_process[type](ins_params, fields);
        }
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
        assert(index < KQT_GENERATORS_MAX);
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
    else if (EVENT_IS_DSP(type))
    {
        assert(index >= 0);
        assert(index < KQT_DSP_EFFECTS_MAX);
        DSP_conf* conf = DSP_table_get_conf(eh->dsps, index);
        if (conf == NULL)
        {
            return false;
        }
        return eh->dsp_process[type](conf, fields);
    }
    return false;
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
    assert(eh != NULL);
    if (eh->global_state != NULL)
    {
//        del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    }
    xfree(eh);
    return;
}


