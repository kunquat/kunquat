

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Param_validator.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <Pat_inst_ref.h>
#include <string/common.h>
#include <string/device_event_name.h>
#include <string/var_name.h>
#include <Value.h>

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


bool v_any_bool(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_BOOL);
}


bool v_any_int(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_INT);
}


bool v_any_float(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_FLOAT);
}


bool v_any_str(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_STRING);
}


bool v_any_ts(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_TSTAMP);
}


bool v_arp_index(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t index = value->value.int_type;

    return (index >= 0) && (index < KQT_ARPEGGIO_TONES_MAX);
}


bool v_arp_speed(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double speed = value->value.float_type;

    return isfinite(speed) && (speed > 0);
}


bool v_au(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t au = value->value.int_type;

    return (au >= 0) && (au < KQT_AUDIO_UNITS_MAX);
}


bool v_cond(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_STRING);
}


bool v_counter(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t counter = value->value.int_type;

    return (counter >= 0) && (counter < 32768);
}


bool v_dev_event_name(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_STRING)
        return false;

    return is_valid_device_event_name(value->value.string_type);
}


bool v_finite_float(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_FLOAT) && isfinite(value->value.float_type);
}


bool v_finite_rt(const Value* value)
{
    rassert(value != NULL);
    return Value_type_is_realtime(value->type) &&
        implies(value->type == VALUE_TYPE_FLOAT, isfinite(value->value.float_type));
}


bool v_force(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double force = value->value.float_type;

    return isfinite(force);
}


bool v_proc(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t proc = value->value.int_type;

    return (proc >= 0) && (proc < KQT_PROCESSORS_MAX);
}


bool v_hit(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t hit = value->value.int_type;

    return (hit >= 0) && (hit < KQT_HITS_MAX);
}


bool v_key(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_STRING)
        return false;

    const char* key = value->value.string_type;

    const size_t len = strlen(key);
    if (len == 0 || len > KQT_KEY_LENGTH_MAX)
        return false;

    for (size_t i = 0; i < len; ++i)
    {
        if (!isalpha(key[i]) && strchr("_./", key[i]) == NULL)
            return false;
    }

    return true;
}


bool v_maybe_finite_rt(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_NONE) || v_finite_rt(value);
}


bool v_maybe_var_name(const Value* value)
{
    rassert(value != NULL);

    if (value->type == VALUE_TYPE_NONE)
        return true;
    if (value->type != VALUE_TYPE_STRING)
        return false;

    const char* str = value->value.string_type;

    return string_eq(str, "") || is_valid_var_name(str);
}


bool v_nonneg_float(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    return (value->value.float_type >= 0);
}


bool v_nonneg_ts(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_TSTAMP)
        return false;

    return (Tstamp_cmp(&value->value.Tstamp_type, Tstamp_set(TSTAMP_AUTO, 0, 0)) >= 0);
}


bool v_note_entry(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t ne = value->value.int_type;

    return (ne >= 0) && (ne < KQT_TUNING_TABLE_NOTES_MAX);
}


bool v_pattern(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t pat = value->value.int_type;

    return (pat >= 0) && (pat < KQT_PATTERNS_MAX);
}


bool v_piref(const Value* value)
{
    rassert(value != NULL);
    return (value->type == VALUE_TYPE_PAT_INST_REF);
}


bool v_pitch(const Value* value)
{
    rassert(value != NULL);
    return v_finite_float(value);
}


bool v_song(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t song = value->value.int_type;

    return (song >= -1) && (song < KQT_SONGS_MAX);
}


bool v_system(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t system = value->value.int_type;

    return (system >= -1);
}


bool v_sustain(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double sustain = value->value.float_type;

    return (sustain >= 0) && (sustain <= 1);
}


bool v_tempo(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double tempo = value->value.float_type;

    return (tempo >= 1) && (tempo <= 999);
}


bool v_track(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t track = value->value.int_type;

    return (track >= -1) && (track < KQT_TRACKS_MAX);
}


bool v_tremolo_depth(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double depth = value->value.float_type;

    return (depth >= 0) && (depth <= 24);
}


bool v_tuning_table(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_INT)
        return false;

    const int64_t tt_index = value->value.int_type;

    return (tt_index >= 0) && (tt_index < KQT_TUNING_TABLES_MAX);
}


bool v_var_name(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_STRING)
        return false;

    return is_valid_var_name(value->value.string_type);
}


bool v_volume(const Value* value)
{
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_FLOAT)
        return false;

    const double vol = value->value.float_type;

    return (vol <= 0);
}


