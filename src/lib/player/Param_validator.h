

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PARAM_VALIDATOR_H
#define KQT_PARAM_VALIDATOR_H


#include <stdbool.h>


typedef bool (*Param_validator)(const char* param);


bool v_any_bool(const char* param);
bool v_any_int(const char* param);
bool v_any_float(const char* param);
bool v_any_str(const char* param);
bool v_any_ts(const char* param);

bool v_arp_index(const char* param);
bool v_arp_speed(const char* param);

bool v_au(const char* param);

bool v_cond(const char* param);

bool v_counter(const char* param);

bool v_finite_float(const char* param);
bool v_finite_rt(const char* param);

bool v_force(const char* param);

bool v_proc(const char* param);

bool v_hit(const char* param);

bool v_key(const char* param);

bool v_maybe_var_name(const char* param);

bool v_nonneg_float(const char* param);
bool v_nonneg_ts(const char* param);

bool v_note_entry(const char* param);

bool v_pattern(const char* param);
bool v_piref(const char* param);

bool v_pitch(const char* param);

bool v_song(const char* param);

bool v_system(const char* param);

bool v_sustain(const char* param);

bool v_tempo(const char* param);

bool v_track(const char* param);

bool v_tremolo_depth(const char* param);

bool v_tuning_table(const char* param);

bool v_var_name(const char* param);

bool v_volume(const char* param);


#endif // KQT_PARAM_VALIDATOR_H


