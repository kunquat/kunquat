

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <File_base.h>
#include <kunquat/limits.h>
#include <Param_validator.h>
#include <Pat_inst_ref.h>
#include <xassert.h>


#define begin() Read_state* state = READ_STATE_AUTO; \
                param = read_const_char(param, '[', state)

#define end() param = read_const_char(param, ']', state)


bool v_any_bool(char* param)
{
    assert(param != NULL);
    begin();
    param = read_bool(param, NULL, state);
    end();
    return !state->error;
}


bool v_any_int(char* param)
{
    assert(param != NULL);
    begin();
    param = read_int(param, NULL, state);
    end();
    return !state->error;
}


bool v_any_float(char* param)
{
    assert(param != NULL);
    begin();
    param = read_double(param, NULL, state);
    end();
    return !state->error;
}


bool v_any_str(char* param)
{
    assert(param != NULL);
    begin();
    param = read_string(param, NULL, 0, state);
    end();
    return !state->error;
}


bool v_any_ts(char* param)
{
    assert(param != NULL);
    begin();
    param = read_reltime(param, NULL, state);
    end();
    return !state->error;
}


bool v_arp_index(char* param)
{
    assert(param != NULL);
    begin();
    int64_t index = -1;
    param = read_int(param, &index, state);
    end();
    return !state->error && index >= 0 && index < KQT_ARPEGGIO_NOTES_MAX;
}


bool v_arp_speed(char* param)
{
    assert(param != NULL);
    begin();
    double speed = NAN;
    param = read_double(param, &speed, state);
    end();
    return !state->error && speed > 0;
}


bool v_cond(char* param)
{
    assert(param != NULL);
    begin();
    param = read_string(param, NULL, 0, state);
    end();
    return !state->error;
}


bool v_counter(char* param)
{
    assert(param != NULL);
    begin();
    int64_t counter = -1;
    param = read_int(param, &counter, state);
    end();
    return !state->error && counter >= 0 && counter < 65535;
}


bool v_dsp(char* param)
{
    assert(param != NULL);
    begin();
    int64_t dsp = -1;
    param = read_int(param, &dsp, state);
    end();
    return !state->error && dsp >= 0 && dsp < KQT_DSPS_MAX;
}


bool v_effect(char* param)
{
    assert(param != NULL);
    begin();
    int64_t effect = -1;
    param = read_int(param, &effect, state);
    end();
    return !state->error && effect >= 0 && effect < KQT_EFFECTS_MAX;
}


bool v_finite_float(char* param)
{
    assert(param != NULL);
    begin();
    double value = NAN;
    param = read_double(param, &value, state);
    end();
    return !state->error && isfinite(value);
}


bool v_force(char* param)
{
    assert(param != NULL);
    begin();
    double force = NAN;
    param = read_double(param, &force, state);
    end();
    return !state->error && force <= 18;
}


bool v_gen(char* param)
{
    assert(param != NULL);
    begin();
    int64_t gen = -1;
    param = read_int(param, &gen, state);
    end();
    return !state->error && gen >= 0 && gen < KQT_GENERATORS_MAX;
}


bool v_hit(char* param)
{
    assert(param != NULL);
    begin();
    int64_t hit = -1;
    param = read_int(param, &hit, state);
    end();
    return !state->error && hit >= 0 && hit < KQT_HITS_MAX;
}


bool v_ins(char* param)
{
    assert(param != NULL);
    begin();
    int64_t ins = -1;
    param = read_int(param, &ins, state);
    end();
    return !state->error && ins >= 0 && ins < KQT_INSTRUMENTS_MAX;
}


bool v_key(char* param)
{
    assert(param != NULL);
    begin();
    char key[KQT_KEY_LENGTH_MAX + 1] = "";
    param = read_string(param, key, KQT_KEY_LENGTH_MAX + 1, state);
    end();
    if (state->error)
    {
        return false;
    }
    int len = strlen(key);
    if (len == 0 || len > KQT_KEY_LENGTH_MAX)
    {
        return false;
    }
    for (int i = 0; i < len; ++i)
    {
        if (!isalpha(key[i]) && strchr("_./", key[i]) == NULL)
        {
            return false;
        }
    }
    return true;
}


bool v_lowpass(char* param)
{
    assert(param != NULL);
    return v_finite_float(param); // TODO
}


bool v_nonneg_float(char* param)
{
    assert(param != NULL);
    begin();
    double value = NAN;
    param = read_double(param, &value, state);
    end();
    return !state->error && value >= 0;
}


bool v_nonneg_ts(char* param)
{
    assert(param != NULL);
    begin();
    Reltime* ts = RELTIME_AUTO;
    param = read_reltime(param, ts, state);
    end();
    return !state->error && Reltime_cmp(ts,
                            Reltime_set(RELTIME_AUTO, 0, 0)) >= 0;
}


bool v_note_entry(char* param)
{
    assert(param != NULL);
    begin();
    int64_t ne = -1;
    param = read_int(param, &ne, state);
    end();
    return !state->error && ne >= 0 && ne < KQT_SCALE_NOTES;
}


bool v_panning(char* param)
{
    assert(param != NULL);
    begin();
    double pan = NAN;
    param = read_double(param, &pan, state);
    end();
    return !state->error && pan >= -1 && pan <= 1;
}


bool v_pattern(char* param)
{
    assert(param != NULL);
    begin();
    int64_t pat = -1;
    param = read_int(param, &pat, state);
    end();
    return !state->error && pat >= 0 && pat < KQT_PATTERNS_MAX;
}


bool v_piref(char* param)
{
    assert(param != NULL);
    begin();
    Pat_inst_ref* piref = PAT_INST_REF_AUTO;
    piref->pat = -1;
    param = read_pat_inst_ref(param, piref, state);
    end();
    return !state->error;
}


bool v_pitch(char* param)
{
    assert(param != NULL);
    return v_finite_float(param);
}


bool v_resonance(char* param)
{
    assert(param != NULL);
    begin();
    double res = NAN;
    param = read_double(param, &res, state);
    end();
    return !state->error && res >= 0 && res <= 99;
}


bool v_scale(char* param)
{
    assert(param != NULL);
    begin();
    int64_t scale = -1;
    param = read_int(param, &scale, state);
    end();
    return !state->error && scale >= 0 && scale < KQT_SCALES_MAX;
}


bool v_system(char* param)
{
    assert(param != NULL);
    begin();
    int64_t system = -2;
    param = read_int(param, &system, state);
    end();
    return !state->error && system >= -1;
}


bool v_subsong(char* param)
{
    assert(param != NULL);
    begin();
    int64_t subsong = -2;
    param = read_int(param, &subsong, state);
    end();
    return !state->error && subsong >= -1 && subsong < KQT_SONGS_MAX;
}


bool v_sustain(char* param)
{
    assert(param != NULL);
    begin();
    double sustain = NAN;
    param = read_double(param, &sustain, state);
    end();
    return !state->error && sustain >= 0 && sustain <= 1;
}


bool v_tempo(char* param)
{
    assert(param != NULL);
    begin();
    double tempo = NAN;
    param = read_double(param, &tempo, state);
    end();
    return !state->error && tempo >= 1 && tempo <= 999;
}


bool v_track(char* param)
{
    assert(param != NULL);
    begin();
    int64_t track = -2;
    param = read_int(param, &track, state);
    end();
    return !state->error && track >= -1 && track < KQT_TRACKS_MAX;
}


bool v_tremolo_depth(char* param)
{
    assert(param != NULL);
    begin();
    double depth = NAN;
    param = read_double(param, &depth, state);
    end();
    return !state->error && depth >= 0 && depth <= 24;
}


bool v_volume(char* param)
{
    assert(param != NULL);
    begin();
    double vol = NAN;
    param = read_double(param, &vol, state);
    end();
    return !state->error && vol <= 0;
}


#undef begin
#undef end


