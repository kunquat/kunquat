

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
#include <stdbool.h>
#include <string.h>

#include <Event.h>
#include <Event_type.h>

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
#include <Event_general_end_if.h>

#include <Event_general_call_bool.h>
#include <Event_general_call_int.h>
#include <Event_general_call_float.h>

#include <Event_global_set_tempo.h>
#include <Event_global_slide_tempo.h>
#include <Event_global_slide_tempo_length.h>
#include <Event_global_pattern_delay.h>

#include <Event_global_set_volume.h>
#include <Event_global_slide_volume.h>
#include <Event_global_slide_volume_length.h>

#include <Event_global_set_scale.h>
#include <Event_global_set_scale_offset.h>
#include <Event_global_mimic_scale.h>
#include <Event_global_set_scale_fixed_point.h>
#include <Event_global_shift_scale_intervals.h>

#include <Event_global_set_jump_subsong.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_jump.h>

#include <Event_ins_set_sustain.h>

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


typedef Event* (*Event_cons)(Reltime* pos);


static const Event_cons cons[EVENT_LAST] =
{
    [EVENT_CONTROL_PAUSE] = new_Event_control_pause,
    [EVENT_CONTROL_RESUME] = new_Event_control_resume,
    [EVENT_CONTROL_PLAY_PATTERN] = new_Event_control_play_pattern,

    [EVENT_CONTROL_ENV_SET_BOOL_NAME] = new_Event_control_env_set_bool_name,
    [EVENT_CONTROL_ENV_SET_BOOL] = new_Event_control_env_set_bool,
    [EVENT_CONTROL_ENV_SET_INT_NAME] = new_Event_control_env_set_int_name,
    [EVENT_CONTROL_ENV_SET_INT] = new_Event_control_env_set_int,
    [EVENT_CONTROL_ENV_SET_FLOAT_NAME] = new_Event_control_env_set_float_name,
    [EVENT_CONTROL_ENV_SET_FLOAT] = new_Event_control_env_set_float,
    [EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME] =
            new_Event_control_env_set_timestamp_name,
    [EVENT_CONTROL_ENV_SET_TIMESTAMP] = new_Event_control_env_set_timestamp,

    [EVENT_CONTROL_SET_GOTO_ROW] = new_Event_control_set_goto_row,
    [EVENT_CONTROL_SET_GOTO_SECTION] = new_Event_control_set_goto_section,
    [EVENT_CONTROL_SET_GOTO_SUBSONG] = new_Event_control_set_goto_subsong,
    [EVENT_CONTROL_GOTO] = new_Event_control_goto,

    [EVENT_CONTROL_INFINITE] = new_Event_control_infinite,

    [EVENT_CONTROL_RECEIVE_EVENT] = new_Event_control_receive_event,

    [EVENT_GENERAL_COMMENT] = new_Event_general_comment,

    [EVENT_GENERAL_COND] = new_Event_general_cond,
    [EVENT_GENERAL_IF] = new_Event_general_if,
    [EVENT_GENERAL_END_IF] = new_Event_general_end_if,

    [EVENT_GENERAL_SIGNAL] = new_Event_general_comment,
    [EVENT_GENERAL_CALL_BOOL_NAME] = new_Event_general_comment,
    [EVENT_GENERAL_CALL_BOOL] = new_Event_general_call_bool,
    [EVENT_GENERAL_CALL_INT_NAME] = new_Event_general_comment,
    [EVENT_GENERAL_CALL_INT] = new_Event_general_call_int,
    [EVENT_GENERAL_CALL_FLOAT_NAME] = new_Event_general_comment,
    [EVENT_GENERAL_CALL_FLOAT] = new_Event_general_call_float,

    [EVENT_GLOBAL_SET_TEMPO] = new_Event_global_set_tempo,
    [EVENT_GLOBAL_SLIDE_TEMPO] = new_Event_global_slide_tempo,
    [EVENT_GLOBAL_SLIDE_TEMPO_LENGTH] = new_Event_global_slide_tempo_length,
    [EVENT_GLOBAL_PATTERN_DELAY] = new_Event_global_pattern_delay,

    [EVENT_GLOBAL_SET_VOLUME] = new_Event_global_set_volume,
    [EVENT_GLOBAL_SLIDE_VOLUME] = new_Event_global_slide_volume,
    [EVENT_GLOBAL_SLIDE_VOLUME_LENGTH] = new_Event_global_slide_volume_length,

    [EVENT_GLOBAL_SET_SCALE] = new_Event_global_set_scale,
    [EVENT_GLOBAL_SET_SCALE_OFFSET] = new_Event_global_set_scale_offset,
    [EVENT_GLOBAL_MIMIC_SCALE] = new_Event_global_mimic_scale,
    [EVENT_GLOBAL_SET_SCALE_FIXED_POINT] = new_Event_global_set_scale_fixed_point,
    [EVENT_GLOBAL_SHIFT_SCALE_INTERVALS] = new_Event_global_shift_scale_intervals,

    [EVENT_GLOBAL_SET_JUMP_SUBSONG] = new_Event_global_set_jump_subsong,
    [EVENT_GLOBAL_SET_JUMP_SECTION] = new_Event_global_set_jump_section,
    [EVENT_GLOBAL_SET_JUMP_ROW] = new_Event_global_set_jump_row,
    [EVENT_GLOBAL_SET_JUMP_COUNTER] = new_Event_global_set_jump_counter,
    [EVENT_GLOBAL_JUMP] = new_Event_global_jump,

    [EVENT_INS_SET_SUSTAIN] = new_Event_ins_set_sustain,

    [EVENT_CHANNEL_SET_INSTRUMENT] = new_Event_channel_set_instrument,
    [EVENT_CHANNEL_SET_GENERATOR] = new_Event_channel_set_generator,
    [EVENT_CHANNEL_SET_EFFECT] = new_Event_channel_set_effect,
    [EVENT_CHANNEL_SET_GLOBAL_EFFECTS] = new_Event_channel_set_global_effects,
    [EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS] =
            new_Event_channel_set_instrument_effects,
    [EVENT_CHANNEL_SET_DSP] = new_Event_channel_set_dsp,

    [EVENT_CHANNEL_NOTE_ON] = new_Event_channel_note_on,
    [EVENT_CHANNEL_HIT] = new_Event_channel_hit,
    [EVENT_CHANNEL_NOTE_OFF] = new_Event_channel_note_off,

    [EVENT_CHANNEL_SET_FORCE] = new_Event_channel_set_force,
    [EVENT_CHANNEL_SLIDE_FORCE] = new_Event_channel_slide_force,
    [EVENT_CHANNEL_SLIDE_FORCE_LENGTH] = new_Event_channel_slide_force_length,
    [EVENT_CHANNEL_TREMOLO_SPEED] = new_Event_channel_tremolo_speed,
    [EVENT_CHANNEL_TREMOLO_DEPTH] = new_Event_channel_tremolo_depth,
    [EVENT_CHANNEL_TREMOLO_DELAY] = new_Event_channel_tremolo_delay,

    [EVENT_CHANNEL_SLIDE_PITCH] = new_Event_channel_slide_pitch,
    [EVENT_CHANNEL_SLIDE_PITCH_LENGTH] = new_Event_channel_slide_pitch_length,
    [EVENT_CHANNEL_VIBRATO_SPEED] = new_Event_channel_vibrato_speed,
    [EVENT_CHANNEL_VIBRATO_DEPTH] = new_Event_channel_vibrato_depth,
    [EVENT_CHANNEL_VIBRATO_DELAY] = new_Event_channel_vibrato_delay,

    [EVENT_CHANNEL_RESET_ARPEGGIO] = new_Event_channel_reset_arpeggio,
    [EVENT_CHANNEL_SET_ARPEGGIO_NOTE] = new_Event_channel_set_arpeggio_note,
    [EVENT_CHANNEL_SET_ARPEGGIO_INDEX] = new_Event_channel_set_arpeggio_index,
    [EVENT_CHANNEL_SET_ARPEGGIO_SPEED] = new_Event_channel_set_arpeggio_speed,
    [EVENT_CHANNEL_ARPEGGIO_ON] = new_Event_channel_arpeggio_on,
    [EVENT_CHANNEL_ARPEGGIO_OFF] = new_Event_channel_arpeggio_off,

    [EVENT_CHANNEL_SET_LOWPASS] = new_Event_channel_set_lowpass,
    [EVENT_CHANNEL_SLIDE_LOWPASS] = new_Event_channel_slide_lowpass,
    [EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH] = new_Event_channel_slide_lowpass_length,
    [EVENT_CHANNEL_AUTOWAH_SPEED] = new_Event_channel_autowah_speed,
    [EVENT_CHANNEL_AUTOWAH_DEPTH] = new_Event_channel_autowah_depth,
    [EVENT_CHANNEL_AUTOWAH_DELAY] = new_Event_channel_autowah_delay,

    [EVENT_CHANNEL_SET_RESONANCE] = new_Event_channel_set_resonance,

    [EVENT_CHANNEL_SET_PANNING] = new_Event_channel_set_panning,
    [EVENT_CHANNEL_SLIDE_PANNING] = new_Event_channel_slide_panning,
    [EVENT_CHANNEL_SLIDE_PANNING_LENGTH] = new_Event_channel_slide_panning_length,

    [EVENT_CHANNEL_SET_GEN_BOOL_NAME] = new_Event_channel_set_gen_bool_name,
    [EVENT_CHANNEL_SET_GEN_BOOL] = new_Event_channel_set_gen_bool,
    [EVENT_CHANNEL_SET_GEN_INT_NAME] = new_Event_channel_set_gen_int_name,
    [EVENT_CHANNEL_SET_GEN_INT] = new_Event_channel_set_gen_int,
    [EVENT_CHANNEL_SET_GEN_FLOAT_NAME] = new_Event_channel_set_gen_float_name,
    [EVENT_CHANNEL_SET_GEN_FLOAT] = new_Event_channel_set_gen_float,
    [EVENT_CHANNEL_SET_GEN_RELTIME_NAME] = new_Event_channel_set_gen_reltime_name,
    [EVENT_CHANNEL_SET_GEN_RELTIME] = new_Event_channel_set_gen_reltime,

    [EVENT_GENERATOR_SET_BOOL_NAME] = new_Event_generator_set_bool_name,
    [EVENT_GENERATOR_SET_BOOL] = new_Event_generator_set_bool,
    [EVENT_GENERATOR_SET_INT_NAME] = new_Event_generator_set_int_name,
    [EVENT_GENERATOR_SET_INT] = new_Event_generator_set_int,
    [EVENT_GENERATOR_SET_FLOAT_NAME] = new_Event_generator_set_float_name,
    [EVENT_GENERATOR_SET_FLOAT] = new_Event_generator_set_float,
    [EVENT_GENERATOR_SET_RELTIME_NAME] = new_Event_generator_set_reltime_name,
    [EVENT_GENERATOR_SET_RELTIME] = new_Event_generator_set_reltime,

    [EVENT_EFFECT_BYPASS_ON] = new_Event_effect_bypass_on,
    [EVENT_EFFECT_BYPASS_OFF] = new_Event_effect_bypass_off,

    [EVENT_DSP_SET_BOOL_NAME] = new_Event_dsp_set_bool_name,
    [EVENT_DSP_SET_BOOL] = new_Event_dsp_set_bool,
    [EVENT_DSP_SET_INT_NAME] = new_Event_dsp_set_int_name,
    [EVENT_DSP_SET_INT] = new_Event_dsp_set_int,
    [EVENT_DSP_SET_FLOAT_NAME] = new_Event_dsp_set_float_name,
    [EVENT_DSP_SET_FLOAT] = new_Event_dsp_set_float,
    [EVENT_DSP_SET_RELTIME_NAME] = new_Event_dsp_set_reltime_name,
    [EVENT_DSP_SET_RELTIME] = new_Event_dsp_set_reltime,
};


bool Event_type_is_supported(Event_type type)
{
    assert(EVENT_IS_VALID(type));
    return cons[type] != NULL || EVENT_IS_CONTROL(type);
}


Event* new_Event(Event_type type, Reltime* pos)
{
    assert(EVENT_IS_VALID(type));
    assert(pos != NULL);
    assert(cons[type] != NULL);
    return cons[type](pos);
}


Event* new_Event_from_string(char** str, Read_state* state,
                             Event_names* names)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(names != NULL);
    if (state->error)
    {
        return NULL;
    }
    *str = read_const_char(*str, '[', state);
    Reltime* pos = Reltime_init(RELTIME_AUTO);
    *str = read_reltime(*str, pos, state);
    *str = read_const_char(*str, ',', state);
    *str = read_const_char(*str, '[', state);
    char* event_desc = *str - 1;
    char type_str[EVENT_NAME_MAX + 2] = "";
    *str = read_string(*str, type_str, EVENT_NAME_MAX + 2, state);
    *str = read_const_char(*str, ',', state);
    if (state->error)
    {
        return NULL;
    }
    Event_type type = Event_names_get(names, type_str);
    if (!EVENT_IS_TRIGGER(type))
    {
        Read_state_set_error(state, "Invalid or unsupported event type:"
                                    " \"%s\"", type_str);
        return NULL;
    }
    if (!Event_type_is_supported(type))
    {
        Read_state_set_error(state, "Unsupported event type: \"%s\"",
                                    type_str);
        return NULL;
    }
    assert(cons[type] != NULL);
    Event* event = cons[type](pos);
    if (event == NULL)
    {
        return NULL;
    }
    char* fields_start = *str;
    *str = Event_type_get_fields(fields_start, event->field_types,
                                 NULL, state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }
    assert(*str != NULL);
    assert(*str > fields_start);
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }
    event->desc = xcalloc(char, *str - event_desc + 1);
    if (event->desc == NULL)
    {
        del_Event(event);
        return NULL;
    }
    strncpy(event->desc, event_desc, *str - event_desc);
    event->fields = event->desc + (fields_start - event_desc);
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }
    return event;
}


