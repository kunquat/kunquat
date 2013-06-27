

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_TYPE_DEF
#error "EVENT_TYPE_DEF(type) not defined"
#endif


EVENT_TYPE_DEF(set_instrument)
EVENT_TYPE_DEF(set_generator)
EVENT_TYPE_DEF(set_effect)
EVENT_TYPE_DEF(set_global_effects)
EVENT_TYPE_DEF(set_instrument_effects)
EVENT_TYPE_DEF(set_dsp)

EVENT_TYPE_DEF(note_on)
EVENT_TYPE_DEF(hit)
EVENT_TYPE_DEF(note_off)

EVENT_TYPE_DEF(set_force)
EVENT_TYPE_DEF(slide_force)
EVENT_TYPE_DEF(slide_force_length)
EVENT_TYPE_DEF(tremolo_speed)
EVENT_TYPE_DEF(tremolo_depth)
EVENT_TYPE_DEF(tremolo_delay)

EVENT_TYPE_DEF(slide_pitch)
EVENT_TYPE_DEF(slide_pitch_length)
EVENT_TYPE_DEF(vibrato_speed)
EVENT_TYPE_DEF(vibrato_depth)
EVENT_TYPE_DEF(vibrato_delay)

EVENT_TYPE_DEF(reset_arpeggio)
EVENT_TYPE_DEF(set_arpeggio_note)
EVENT_TYPE_DEF(set_arpeggio_index)
EVENT_TYPE_DEF(set_arpeggio_speed)
EVENT_TYPE_DEF(arpeggio_on)
EVENT_TYPE_DEF(arpeggio_off)

EVENT_TYPE_DEF(set_lowpass)
EVENT_TYPE_DEF(slide_lowpass)
EVENT_TYPE_DEF(slide_lowpass_length)
EVENT_TYPE_DEF(autowah_speed)
EVENT_TYPE_DEF(autowah_depth)
EVENT_TYPE_DEF(autowah_delay)

EVENT_TYPE_DEF(set_resonance)
//EVENT_TYPE_DEF(slide_resonance)
//EVENT_TYPE_DEF(slide_resonance_length)

EVENT_TYPE_DEF(set_panning)
EVENT_TYPE_DEF(slide_panning)
EVENT_TYPE_DEF(slide_panning_length)

EVENT_TYPE_DEF(set_gen_bool_name)
EVENT_TYPE_DEF(set_gen_bool)
EVENT_TYPE_DEF(set_gen_int_name)
EVENT_TYPE_DEF(set_gen_int)
EVENT_TYPE_DEF(set_gen_float_name)
EVENT_TYPE_DEF(set_gen_float)
EVENT_TYPE_DEF(set_gen_tstamp_name)
EVENT_TYPE_DEF(set_gen_tstamp)


#undef EVENT_TYPE_DEF


