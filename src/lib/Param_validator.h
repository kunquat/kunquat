

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PARAM_VALIDATOR_H
#define K_PARAM_VALIDATOR_H


#include <stdbool.h>


typedef bool (*Param_validator)(char* param);


bool v_any_bool(char* param);
bool v_any_int(char* param);
bool v_any_float(char* param);
bool v_any_str(char* param);
bool v_any_ts(char* param);

bool v_arp_index(char* param);
bool v_arp_speed(char* param);

bool v_cond(char* param);

bool v_counter(char* param);

bool v_dsp(char* param);

bool v_effect(char* param);

bool v_finite_float(char* param);

bool v_force(char* param);

bool v_gen(char* param);

bool v_hit(char* param);

bool v_ins(char* param);

bool v_key(char* param);

bool v_lowpass(char* param);

bool v_nonneg_float(char* param);
bool v_nonneg_ts(char* param);

bool v_note_entry(char* param);

bool v_panning(char* param);

bool v_pattern(char* param);

bool v_pitch(char* param);

bool v_resonance(char* param);

bool v_scale(char* param);

bool v_section(char* param);

bool v_subsong(char* param);

bool v_sustain(char* param);

bool v_tempo(char* param);

bool v_tremolo_depth(char* param);

bool v_volume(char* param);


#endif // K_PARAM_VALIDATOR_H


