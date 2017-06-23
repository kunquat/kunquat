

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2017
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


#include <Value.h>

#include <stdbool.h>


typedef bool Param_validator(const Value* value);


Param_validator v_any_bool;
Param_validator v_any_int;
Param_validator v_any_float;
Param_validator v_any_str;
Param_validator v_any_ts;

Param_validator v_arp_index;
Param_validator v_arp_speed;

Param_validator v_au;

Param_validator v_cond;

Param_validator v_counter;

Param_validator v_finite_float;
Param_validator v_finite_rt;

Param_validator v_force;

Param_validator v_proc;

Param_validator v_hit;

Param_validator v_key;

Param_validator v_maybe_var_name;

Param_validator v_nonneg_float;
Param_validator v_nonneg_ts;

Param_validator v_note_entry;

Param_validator v_pattern;
Param_validator v_piref;

Param_validator v_pitch;

Param_validator v_song;

Param_validator v_system;

Param_validator v_sustain;

Param_validator v_tempo;

Param_validator v_track;

Param_validator v_tremolo_depth;

Param_validator v_tuning_table;

Param_validator v_var_name;

Param_validator v_volume;


#endif // KQT_PARAM_VALIDATOR_H


