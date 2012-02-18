

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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

#include <Event_control_pause.h>
#include <Event_control_resume.h>
#include <Event_control_play_pattern.h>

#include <Event_control_env_set_bool_name.h>
#include <Event_control_env_set_bool.h>
#include <Event_control_env_set_int_name.h>
#include <Event_control_env_set_int.h>
#include <Event_control_env_set_float_name.h>
#include <Event_control_env_set_float.h>
#include <Event_control_env_set_timestamp_name.h>
#include <Event_control_env_set_timestamp.h>

#include <Event_control_set_goto_row.h>
#include <Event_control_set_goto_section.h>
#include <Event_control_set_goto_subsong.h>
#include <Event_control_goto.h>

#include <Event_control_infinite.h>

#include <Event_control_receive_event.h>

#include <Event_general_comment.h>

#include <Event_general_cond.h>
#include <Event_general_if.h>
#include <Event_general_else.h>
#include <Event_general_end_if.h>

#include <Event_general_call_bool.h>
#include <Event_general_call_int.h>
#include <Event_general_call_float.h>

#include <Event_global_pattern_delay.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_subsong.h>
//#include <Event_global_jump.h>

#include <Event_global_set_scale.h>
#include <Event_global_set_scale_offset.h>
#include <Event_global_mimic_scale.h>
#include <Event_global_set_scale_fixed_point.h>
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
#include <Event_channel_set_global_effects.h>
#include <Event_channel_set_instrument_effects.h>
#include <Event_channel_set_dsp.h>

#include <Event_channel_note_on.h>
#include <Event_channel_hit.h>
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

#include <Event_channel_reset_arpeggio.h>
#include <Event_channel_set_arpeggio_note.h>
#include <Event_channel_set_arpeggio_index.h>
#include <Event_channel_set_arpeggio_speed.h>
#include <Event_channel_arpeggio_on.h>
#include <Event_channel_arpeggio_off.h>

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

#include <Event_channel_set_gen_bool_name.h>
#include <Event_channel_set_gen_bool.h>
#include <Event_channel_set_gen_int_name.h>
#include <Event_channel_set_gen_int.h>
#include <Event_channel_set_gen_float_name.h>
#include <Event_channel_set_gen_float.h>
#include <Event_channel_set_gen_reltime_name.h>
#include <Event_channel_set_gen_reltime.h>

#include <Event_ins_set_sustain.h>

#include <Event_generator_set_bool_name.h>
#include <Event_generator_set_bool.h>
#include <Event_generator_set_int_name.h>
#include <Event_generator_set_int.h>
#include <Event_generator_set_float_name.h>
#include <Event_generator_set_float.h>
#include <Event_generator_set_reltime_name.h>
#include <Event_generator_set_reltime.h>

#include <Event_effect_bypass_on.h>
#include <Event_effect_bypass_off.h>

#include <Event_dsp_set_bool_name.h>
#include <Event_dsp_set_bool.h>
#include <Event_dsp_set_int_name.h>
#include <Event_dsp_set_int.h>
#include <Event_dsp_set_float_name.h>
#include <Event_dsp_set_float.h>
#include <Event_dsp_set_reltime_name.h>
#include <Event_dsp_set_reltime.h>

#include <xassert.h>
#include <xmemory.h>


struct Event_handler
{
    bool mute; // FIXME: this is just to make the stupid Channel_state_init happy
    Channel_state* ch_states[KQT_COLUMNS_MAX];
    Ins_table* insts;
    Effect_table* effects;
    Playdata* global_state;
    Event_names* event_names;
    Event_buffer* event_buffer;
    Event_buffer* tracker_buffer;
    bool (*control_process[EVENT_CONTROL_UPPER])(General_state*, Value*);
    bool (*general_process[EVENT_GENERAL_UPPER])(General_state*, Value*);
    bool (*ch_process[EVENT_CHANNEL_UPPER])(Channel_state*, Value*);
    bool (*global_process[EVENT_GLOBAL_UPPER])(Playdata*, Value*);
    bool (*ins_process[EVENT_INS_UPPER])(Instrument_params*, Value*);
    bool (*generator_process[EVENT_GENERATOR_UPPER])(Generator*,
                                                     Channel_state*, Value*);
    bool (*effect_process[EVENT_EFFECT_UPPER])(Effect*, Value*);
    bool (*dsp_process[EVENT_DSP_UPPER])(DSP_conf*, Channel_state*, Value*);
};


static bool Event_handler_handle(Event_handler* eh,
                                 int index,
                                 Event_type type,
                                 Value* value);


Event_handler* new_Event_handler(Playdata* global_state,
                                 Channel_state** ch_states,
                                 Ins_table* insts,
                                 Effect_table* effects)
{
    assert(global_state != NULL);
    assert(ch_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);
    Event_handler* eh = xalloc(Event_handler);
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

    Event_handler_set_control_process(eh, EVENT_CONTROL_PAUSE,
                                      Event_control_pause_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_RESUME,
                                      Event_control_resume_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_PLAY_PATTERN,
                                      Event_control_play_pattern_process);

    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_BOOL_NAME,
                                      Event_control_env_set_bool_name_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_BOOL,
                                      Event_control_env_set_bool_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_INT_NAME,
                                      Event_control_env_set_int_name_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_INT,
                                      Event_control_env_set_int_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_FLOAT_NAME,
                                      Event_control_env_set_float_name_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_FLOAT,
                                      Event_control_env_set_float_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME,
                                      Event_control_env_set_timestamp_name_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_ENV_SET_TIMESTAMP,
                                      Event_control_env_set_timestamp_process);

    Event_handler_set_control_process(eh, EVENT_CONTROL_SET_GOTO_ROW,
                                      Event_control_set_goto_row_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_SET_GOTO_SECTION,
                                      Event_control_set_goto_section_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_SET_GOTO_SUBSONG,
                                      Event_control_set_goto_subsong_process);
    Event_handler_set_control_process(eh, EVENT_CONTROL_GOTO,
                                      Event_control_goto_process);

    Event_handler_set_control_process(eh, EVENT_CONTROL_INFINITE,
                                      Event_control_infinite_process);

    Event_handler_set_control_process(eh, EVENT_CONTROL_RECEIVE_EVENT,
                                      Event_control_receive_event);

    Event_handler_set_general_process(eh, EVENT_GENERAL_COMMENT,
                                      Event_general_comment_process);

    Event_handler_set_general_process(eh, EVENT_GENERAL_COND,
                                      Event_general_cond_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_IF,
                                      Event_general_if_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_ELSE,
                                      Event_general_else_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_END_IF,
                                      Event_general_end_if_process);

    Event_handler_set_general_process(eh, EVENT_GENERAL_SIGNAL,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_BOOL_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_BOOL,
                                      Event_general_call_bool_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_INT_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_INT,
                                      Event_general_call_int_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_FLOAT_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, EVENT_GENERAL_CALL_FLOAT,
                                      Event_general_call_float_process);

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
    //Event_handler_set_global_process(eh, EVENT_GLOBAL_JUMP,
    //                                 Event_global_jump_process);

    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_SCALE,
                                     Event_global_set_scale_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_SCALE_OFFSET,
                                     Event_global_set_scale_offset_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_MIMIC_SCALE,
                                     Event_global_mimic_scale_process);
    Event_handler_set_global_process(eh, EVENT_GLOBAL_SET_SCALE_FIXED_POINT,
                                     Event_global_set_scale_fixed_point_process);
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
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_EFFECT,
                                 Event_channel_set_effect_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GLOBAL_EFFECTS,
                                 Event_channel_set_global_effects_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS,
                                 Event_channel_set_instrument_effects_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_DSP,
                                 Event_channel_set_dsp_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_NOTE_ON,
                                 Event_channel_note_on_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_HIT,
                                 Event_channel_hit_process);
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

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_RESET_ARPEGGIO,
                                 Event_channel_reset_arpeggio_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_ARPEGGIO_NOTE,
                                 Event_channel_set_arpeggio_note_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_ARPEGGIO_INDEX,
                                 Event_channel_set_arpeggio_index_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_ARPEGGIO_SPEED,
                                 Event_channel_set_arpeggio_speed_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_ARPEGGIO_ON,
                                 Event_channel_arpeggio_on_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_ARPEGGIO_OFF,
                                 Event_channel_arpeggio_off_process);

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_LOWPASS,
                                 Event_channel_set_lowpass_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_LOWPASS,
                                 Event_channel_slide_lowpass_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH,
                                 Event_channel_slide_lowpass_length_process);
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

    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_BOOL_NAME,
                                 Event_channel_set_gen_bool_name_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_BOOL,
                                 Event_channel_set_gen_bool_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_INT_NAME,
                                 Event_channel_set_gen_int_name_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_INT,
                                 Event_channel_set_gen_int_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_FLOAT_NAME,
                                 Event_channel_set_gen_float_name_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_FLOAT,
                                 Event_channel_set_gen_float_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_RELTIME_NAME,
                                 Event_channel_set_gen_reltime_name_process);
    Event_handler_set_ch_process(eh, EVENT_CHANNEL_SET_GEN_RELTIME,
                                 Event_channel_set_gen_reltime_process);

    Event_handler_set_ins_process(eh, EVENT_INS_SET_SUSTAIN,
                                  Event_ins_set_sustain_process);

    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_BOOL_NAME,
                                        Event_generator_set_bool_name_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_BOOL,
                                        Event_generator_set_bool_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_INT_NAME,
                                        Event_generator_set_int_name_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_INT,
                                        Event_generator_set_int_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_FLOAT_NAME,
                                        Event_generator_set_float_name_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_FLOAT,
                                        Event_generator_set_float_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_RELTIME_NAME,
                                        Event_generator_set_reltime_name_process);
    Event_handler_set_generator_process(eh, EVENT_GENERATOR_SET_RELTIME,
                                        Event_generator_set_reltime_process);

    Event_handler_set_effect_process(eh, EVENT_EFFECT_BYPASS_ON,
                                     Event_effect_bypass_on_process);
    Event_handler_set_effect_process(eh, EVENT_EFFECT_BYPASS_OFF,
                                     Event_effect_bypass_off_process);

    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_BOOL_NAME,
                                  Event_dsp_set_bool_name_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_BOOL,
                                  Event_dsp_set_bool_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_INT_NAME,
                                  Event_dsp_set_int_name_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_INT,
                                  Event_dsp_set_int_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_FLOAT_NAME,
                                  Event_dsp_set_float_name_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_FLOAT,
                                  Event_dsp_set_float_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_RELTIME_NAME,
                                  Event_dsp_set_reltime_name_process);
    Event_handler_set_dsp_process(eh, EVENT_DSP_SET_RELTIME,
                                  Event_dsp_set_reltime_process);

#if 0
    if (Event_names_error(eh->event_names))
    {
        del_Event_handler(eh);
        return NULL;
    }
#endif
    Playdata_set_event_filter(global_state, eh->event_names);
    return eh;
}


Event_names* Event_handler_get_names(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(Event_handler* eh,
//                                  const char* name,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_CHANNEL(type));
    assert(ch_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->ch_process[type] = ch_process;
    return true;
}


bool Event_handler_set_general_process(Event_handler* eh,
//                                       const char* name,
                                       Event_type type,
                                       bool (*general_process)(General_state*,
                                                               Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_GENERAL(type));
    assert(general_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->general_process[type] = general_process;
    return true;
}


bool Event_handler_set_control_process(Event_handler* eh,
//                                       const char* name,
                                       Event_type type,
                                       bool (*control_process)(General_state*,
                                                               Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_CONTROL(type));
    assert(control_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->control_process[type] = control_process;
    return true;
}


bool Event_handler_set_global_process(Event_handler* eh,
//                                      const char* name,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_GLOBAL(type));
    assert(global_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->global_process[type] = global_process;
    return true;
}


bool Event_handler_set_ins_process(Event_handler* eh,
//                                   const char* name,
                                   Event_type type,
                                   bool (*ins_process)(Instrument_params*,
                                                       Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_INS(type));
    assert(ins_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->ins_process[type] = ins_process;
    return true;
}


bool Event_handler_set_generator_process(Event_handler* eh,
//                                         const char* name,
                                         Event_type type,
                                         bool (*gen_process)(Generator*,
                                                             Channel_state*,
                                                             Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_GENERATOR(type));
    assert(gen_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->generator_process[type] = gen_process;
    return true;
}


bool Event_handler_set_effect_process(Event_handler* eh,
//                                      const char* name,
                                      Event_type type,
                                      bool (*effect_process)(Effect*, Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_EFFECT(type));
    assert(effect_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->effect_process[type] = effect_process;
    return true;
}


bool Event_handler_set_dsp_process(Event_handler* eh,
//                                   const char* name,
                                   Event_type type,
                                   bool (*dsp_process)(DSP_conf*,
                                                       Channel_state*,
                                                       Value*))
{
    assert(eh != NULL);
#if 0
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
#endif
    assert(EVENT_IS_DSP(type));
    assert(dsp_process != NULL);
#if 0
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
#endif
    eh->dsp_process[type] = dsp_process;
    return true;
}


static bool Event_handler_handle(Event_handler* eh,
                                 int index,
                                 Event_type type,
                                 Value* value)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(EVENT_IS_VALID(type));
    if (EVENT_IS_CHANNEL(type))
    {
//        assert(index >= 0);
        if (eh->ch_process[type] == NULL)
        {
            return false;
        }
        return eh->ch_process[type](eh->ch_states[index], value);
    }
    else if (EVENT_IS_INS(type))
    {
//        assert(index >= 0);
//        Instrument* ins = Ins_table_get(eh->insts, index);
        Instrument* ins = Ins_table_get(eh->insts,
                                        eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        return eh->ins_process[type](ins_params, value);
    }
    else if (EVENT_IS_GLOBAL(type))
    {
//        assert(index == -1);
        if (eh->global_process[type] == NULL)
        {
            return false;
        }
        return eh->global_process[type](eh->global_state, value);
    }
    else if (EVENT_IS_GENERATOR(type))
    {
//        assert(index >= 0);
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
        return eh->generator_process[type](gen, eh->ch_states[index], value);
    }
    else if (EVENT_IS_EFFECT(type))
    {
//        assert(index >= 0);
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
        return eh->effect_process[type](eff, value);
    }
    else if (EVENT_IS_DSP(type))
    {
//        assert(index >= 0);
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
        return eh->dsp_process[type](conf, eh->ch_states[index], value);
    }
    else if (EVENT_IS_CONTROL(type))
    {
        return eh->control_process[type]((General_state*)eh->global_state,
                                         value);
    }
    else if (EVENT_IS_GENERAL(type))
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


static void Event_handler_handle_query(Event_handler* eh,
                                       int index,
                                       Event_type event_type,
                                       Value* event_arg,
                                       bool silent);


static void Event_handler_handle_query(Event_handler* eh,
                                       int index,
                                       Event_type event_type,
                                       Value* event_arg,
                                       bool silent)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(EVENT_IS_QUERY(event_type));
    assert(event_arg != NULL);
    char auto_event[128] = "";
    switch (event_type)
    {
        case EVENT_QUERY_LOCATION:
        {
            snprintf(auto_event, 128, "[\"Asubsong\", %" PRIu16 "]",
                     eh->global_state->subsong);
            Event_handler_trigger_const(eh, index, auto_event, silent);
            snprintf(auto_event, 128, "[\"Asection\", %" PRIu16 "]",
                     eh->global_state->section);
            Event_handler_trigger_const(eh, index, auto_event, silent);
            snprintf(auto_event, 128, "[\"Apattern\", %" PRId16 "]",
                     eh->global_state->pattern);
            Event_handler_trigger_const(eh, index, auto_event, silent);
            snprintf(auto_event, 128,
                     "[\"Arow\", [%" PRId64 ", %" PRId32 "]]",
                     Reltime_get_beats(&eh->global_state->pos),
                     Reltime_get_rem(&eh->global_state->pos));
            Event_handler_trigger_const(eh, index, auto_event, silent);
        } break;
        case EVENT_QUERY_VOICE_COUNT:
        {
            snprintf(auto_event, 128, "[\"Avoices\", %d]",
                     eh->global_state->active_voices);
            eh->global_state->active_voices = 0;
            Event_handler_trigger_const(eh, index, auto_event, silent);
        } break;
        default:
            assert(false);
    }
    return;
}


static bool Event_handler_act(Event_handler* eh,
                              bool silent,
                              int index,
                              char* event_name,
                              Event_type event_type,
                              Value* value);


static bool Event_handler_act(Event_handler* eh,
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
#if 0
    Read_state* state = READ_STATE_AUTO;
    char* str = read_const_char(desc, '[', state);
    char event_name[EVENT_NAME_MAX + 2] = "";
    str = read_string(str, event_name, EVENT_NAME_MAX + 2, state);
    str = read_const_char(str, ',', state);
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
#if 0
    if ((EVENT_IS_GLOBAL(type) != (index == -1)) &&
            !EVENT_IS_GENERAL(type) && !EVENT_IS_CONTROL(type))
    {
        return false;
    }
#endif
    if (!General_state_events_enabled((General_state*)eh->ch_states[index]) &&
            type != EVENT_GENERAL_IF && type != EVENT_GENERAL_ELSE &&
            type != EVENT_GENERAL_END_IF)
    {
        return true;
    }
#endif
#if 0
    Value* value = VALUE_AUTO;
    Event_field_type field_type = Event_names_get_param_type(eh->event_names,
                                                             event_name);
    //str = read_const_char(str, '[', state);
    //if (state->error)
    //{
    //    return false;
    //}
    if (field_type == EVENT_FIELD_NONE)
    {
        value->type = VALUE_TYPE_NONE;
        str = read_null(str, state);
    }
    else
    {
        char* quote_pos = strrchr(event_name, '"');
        if (quote_pos != NULL && string_eq(quote_pos, "\""))
        {
            value->type = VALUE_TYPE_STRING;
            str = read_string(str, value->value.string_type,
                              ENV_VAR_NAME_MAX, state);
        }
        else
        {
            str = evaluate_expr(str, eh->global_state->parent.env, state,
                                meta, value);
        }
        if (state->error)
        {
            return false;
        }
        switch (field_type)
        {
            case EVENT_FIELD_BOOL:
            {
                if (value->type != VALUE_TYPE_BOOL)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case EVENT_FIELD_INT:
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
            case EVENT_FIELD_DOUBLE:
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
            case EVENT_FIELD_REAL:
            {
                assert(false);
            } break;
            case EVENT_FIELD_RELTIME:
            {
                if (value->type == VALUE_TYPE_INT)
                {
                    value->type = VALUE_TYPE_TIMESTAMP;
                    Reltime_set(&value->value.Timestamp_type,
                                value->value.int_type, 0);
                }
                else if (value->type == VALUE_TYPE_FLOAT)
                {
                    value->type = VALUE_TYPE_TIMESTAMP;
                    double beats = floor(value->value.float_type);
                    Reltime_set(&value->value.Timestamp_type, beats,
                                (value->value.float_type - beats) *
                                    KQT_RELTIME_BEAT);
                }
                else if (value->type != VALUE_TYPE_TIMESTAMP)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case EVENT_FIELD_STRING:
            {
                if (value->type != VALUE_TYPE_STRING)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            default:
                assert(false);
        }
    }
#endif
    //str = read_const_char(str, ']', state);
    //if (state->error)
    //{
    //    return false;
    //}
    if (!EVENT_IS_QUERY(event_type) && !EVENT_IS_AUTO(event_type) &&
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
        if ((event_type >= EVENT_CONTROL_ENV_SET_BOOL_NAME &&
                event_type <= EVENT_CONTROL_ENV_SET_TIMESTAMP) ||
            EVENT_IS_AUTO(event_type))
        {
            Event_buffer_add(eh->tracker_buffer, index, event_name, value);
        }
    }
    if (EVENT_IS_QUERY(event_type))
    {
        Event_handler_handle_query(eh, index, event_type, value, silent);
    }
    Target_event* bound = Bind_get_first(eh->global_state->bind,
                                         eh->ch_states[index]->event_cache,
                                         eh->global_state->parent.env,
                                         event_name,
                                         value,
                                         eh->ch_states[index]->rand);
    while (bound != NULL)
    {
        Event_handler_trigger(eh,
                              (index + bound->ch_offset +
                                      KQT_COLUMNS_MAX) %
                                      KQT_COLUMNS_MAX,
                              bound->desc,
                              silent,
                              value);
        bound = bound->next;
    }
    return true;
}


bool Event_handler_process_type(Event_handler* eh,
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
    if (*event_type == EVENT_NONE)
    {
        Read_state_set_error(state, "Unsupported event type: %s",
                                    event_name);
        return false;
    }
    assert(Event_type_is_supported(*event_type) ||
           EVENT_IS_QUERY(*event_type) || EVENT_IS_AUTO(*event_type));
    if (!General_state_events_enabled((General_state*)eh->ch_states[index]) &&
            *event_type != EVENT_GENERAL_IF &&
            *event_type != EVENT_GENERAL_ELSE &&
            *event_type != EVENT_GENERAL_END_IF)
    {
        return false;
    }
    return true;
}


bool Event_handler_trigger(Event_handler* eh,
                           int index,
                           char* desc,
                           bool silent,
                           Value* meta)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    Read_state* state = READ_STATE_AUTO;
    char event_name[EVENT_NAME_MAX + 2] = "";
    Event_type event_type = EVENT_NONE;
    if (!Event_handler_process_type(eh, index, &desc,
                                    event_name, &event_type, state))
    {
        return !state->error;
    }
    assert(!state->error);
    Value* value = VALUE_AUTO;
    Event_field_type field_type = Event_names_get_param_type(eh->event_names,
                                                             event_name);
    if (field_type == EVENT_FIELD_NONE)
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
            desc = evaluate_expr(desc, eh->global_state->parent.env, state,
                                 meta, value, eh->ch_states[index]->rand);
            desc = read_const_char(desc, '"', state);
        }
        switch (field_type)
        {
            case EVENT_FIELD_BOOL:
            {
                if (value->type != VALUE_TYPE_BOOL)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case EVENT_FIELD_INT:
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
            case EVENT_FIELD_DOUBLE:
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
            case EVENT_FIELD_REAL:
            {
                assert(false);
            } break;
            case EVENT_FIELD_RELTIME:
            {
                if (value->type == VALUE_TYPE_INT)
                {
                    value->type = VALUE_TYPE_TIMESTAMP;
                    Reltime_set(&value->value.Timestamp_type,
                                value->value.int_type, 0);
                }
                else if (value->type == VALUE_TYPE_FLOAT)
                {
                    value->type = VALUE_TYPE_TIMESTAMP;
                    double beats = floor(value->value.float_type);
                    Reltime_set(&value->value.Timestamp_type, beats,
                                (value->value.float_type - beats) *
                                    KQT_RELTIME_BEAT);
                }
                else if (value->type != VALUE_TYPE_TIMESTAMP)
                {
                    Read_state_set_error(state, "Type mismatch");
                    return false;
                }
            } break;
            case EVENT_FIELD_STRING:
            {
                if (value->type != VALUE_TYPE_STRING)
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
    return Event_handler_act(eh, silent, index,
                             event_name, event_type, value);
}


bool Event_handler_trigger_const(Event_handler* eh,
                                 int index,
                                 char* desc,
                                 bool silent)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    Read_state* state = READ_STATE_AUTO;
    char event_name[EVENT_NAME_MAX + 2] = "";
    Event_type event_type = EVENT_NONE;
    if (!Event_handler_process_type(eh, index, &desc,
                                    event_name, &event_type, state))
    {
        return !state->error;
    }
    assert(!state->error);
    Value* value = VALUE_AUTO;
    Event_field_type field_type = Event_names_get_param_type(eh->event_names,
                                                             event_name);
    switch (field_type)
    {
        case EVENT_FIELD_NONE:
        {
            value->type = VALUE_TYPE_NONE;
            desc = read_null(desc, state);
        } break;
        case EVENT_FIELD_BOOL:
        {
            value->type = VALUE_TYPE_BOOL;
            desc = read_bool(desc, &value->value.bool_type, state);
        } break;
        case EVENT_FIELD_INT:
        {
            value->type = VALUE_TYPE_INT;
            desc = read_int(desc, &value->value.int_type, state);
        } break;
        case EVENT_FIELD_DOUBLE:
        {
            value->type = VALUE_TYPE_FLOAT;
            desc = read_double(desc, &value->value.float_type, state);
        } break;
        case EVENT_FIELD_REAL:
        {
            assert(false);
        } break;
        case EVENT_FIELD_RELTIME:
        {
            value->type = VALUE_TYPE_TIMESTAMP;
            desc = read_reltime(desc, &value->value.Timestamp_type, state);
        } break;
        case EVENT_FIELD_STRING:
        {
            value->type = VALUE_TYPE_STRING;
            desc = read_string(desc, value->value.string_type,
                               ENV_VAR_NAME_MAX, state);
        }
        default:
            assert(false);
    }
    desc = read_const_char(desc, ']', state);
    if (state->error)
    {
        return false;
    }
    return Event_handler_act(eh, silent, index,
                             event_name, event_type, value);
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
    del_Event_buffer(eh->event_buffer);
    del_Event_buffer(eh->tracker_buffer);
//    del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    xfree(eh);
    return;
}


