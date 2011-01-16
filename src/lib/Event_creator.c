

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
#include <stdbool.h>

#include <Event.h>
#include <Event_type.h>

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
#include <Event_global_shift_scale_intervals.h>

#include <Event_global_set_jump_subsong.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_jump.h>


#include <Event_ins_set_pedal.h>

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


typedef Event* (*Event_cons)(Reltime* pos);


static const Event_cons cons[EVENT_LAST] =
{
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
    [EVENT_GLOBAL_SHIFT_SCALE_INTERVALS] = new_Event_global_shift_scale_intervals,

    [EVENT_GLOBAL_SET_JUMP_SUBSONG] = new_Event_global_set_jump_subsong,
    [EVENT_GLOBAL_SET_JUMP_SECTION] = new_Event_global_set_jump_section,
    [EVENT_GLOBAL_SET_JUMP_ROW] = new_Event_global_set_jump_row,
    [EVENT_GLOBAL_SET_JUMP_COUNTER] = new_Event_global_set_jump_counter,
    [EVENT_GLOBAL_JUMP] = new_Event_global_jump,

    [EVENT_INS_SET_PEDAL] = new_Event_ins_set_pedal,

    [EVENT_CHANNEL_SET_INSTRUMENT] = new_Event_channel_set_instrument,
    [EVENT_CHANNEL_SET_GENERATOR] = new_Event_channel_set_generator,
    [EVENT_CHANNEL_SET_EFFECT] = new_Event_channel_set_effect,
    [EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS] =
            new_Event_channel_set_instrument_effects,
    [EVENT_CHANNEL_SET_DSP] = new_Event_channel_set_dsp,
    [EVENT_CHANNEL_SET_DSP_CONTEXT] = new_Event_channel_set_dsp_context,

    [EVENT_CHANNEL_NOTE_ON] = new_Event_channel_note_on,
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
    [EVENT_CHANNEL_ARPEGGIO] = new_Event_channel_arpeggio,

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

    [EVENT_CHANNEL_SET_GEN_BOOL] = new_Event_channel_set_gen_bool,
    [EVENT_CHANNEL_SET_GEN_INT] = new_Event_channel_set_gen_int,
    [EVENT_CHANNEL_SET_GEN_FLOAT] = new_Event_channel_set_gen_float,
    [EVENT_CHANNEL_SET_GEN_RELTIME] = new_Event_channel_set_gen_reltime,

    [EVENT_GENERATOR_SET_BOOL] = new_Event_generator_set_bool,
    [EVENT_GENERATOR_SET_INT] = new_Event_generator_set_int,
    [EVENT_GENERATOR_SET_FLOAT] = new_Event_generator_set_float,
    [EVENT_GENERATOR_SET_RELTIME] = new_Event_generator_set_reltime,

    [EVENT_EFFECT_ENABLE] = new_Event_effect_enable,
    [EVENT_EFFECT_DISABLE] = new_Event_effect_disable,

    [EVENT_DSP_SET_BOOL] = new_Event_dsp_set_bool,
    [EVENT_DSP_SET_INT] = new_Event_dsp_set_int,
    [EVENT_DSP_SET_FLOAT] = new_Event_dsp_set_float,
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


