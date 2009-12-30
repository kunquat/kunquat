

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <Scale.h>
#include <File_base.h>
#include <File_tree.h>
#include <math_common.h>

#include <xmemory.h>


#define NOTE_EXISTS(scale, index) (Real_get_numerator(&(scale)->notes[(index)].ratio) >= 0)
#define NOTE_MOD_EXISTS(scale, index) (Real_get_numerator(&(scale)->note_mods[(index)].ratio) >= 0)

#define NOTE_CLEAR(scale, index)                                          \
    if (true)                                                             \
    {                                                                     \
        (scale)->notes[(index)].cents = NAN;                              \
        Real_init_as_frac(&(scale)->notes[(index)].ratio, -1, 1);         \
        Real_init_as_frac(&(scale)->notes[(index)].ratio_retuned, -1, 1); \
    } else (void)0

#define NOTE_MOD_CLEAR(scale, index)                                  \
    if (true)                                                         \
    {                                                                 \
        (scale)->note_mods[(index)].cents = NAN;                      \
        Real_init_as_frac(&(scale)->note_mods[(index)].ratio, -1, 1); \
    } else (void)0

static Scale* Scale_init(Scale* scale, pitch_t ref_pitch, Real* octave_ratio);

static Scale* Scale_copy(Scale* dest, const Scale* src);


Scale* new_Scale(pitch_t ref_pitch, Real* octave_ratio)
{
    assert(ref_pitch > 0);
    assert(octave_ratio != NULL);
    assert( Real_cmp(octave_ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    Scale* scale = xalloc(Scale);
    if (scale == NULL)
    {
        return NULL;
    }
    return Scale_init(scale, ref_pitch, octave_ratio);
}


static Scale* Scale_init(Scale* scale, pitch_t ref_pitch, Real* octave_ratio)
{
    assert(scale != NULL);
    assert(octave_ratio != NULL);
    assert( Real_cmp(octave_ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    Scale_clear(scale);
    scale->ref_pitch = ref_pitch;
    Scale_set_octave_ratio(scale, octave_ratio);
    scale->oct_ratio_cents = NAN;
    return scale;
}

static Scale* Scale_copy(Scale* dest, const Scale* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    return memcpy(dest, src, sizeof(Scale));
}


#define read_and_validate_tuning(str, ratio, cents, state)                        \
    if (true)                                                                     \
    {                                                                             \
        (str) = read_tuning((str), (ratio), &(cents), (state));                   \
        if ((state)->error)                                                       \
        {                                                                         \
            return false;                                                         \
        }                                                                         \
        if (isnan((cents)))                                                       \
        {                                                                         \
            if ((Real_is_frac((ratio)) && Real_get_numerator((ratio)) <= 0)       \
                    || (!Real_is_frac((ratio)) && Real_get_double((ratio)) <= 0)) \
            {                                                                     \
                Read_state_set_error((state), "Ratio is not positive");           \
                return false;                                                     \
            }                                                                     \
        }                                                                         \
    } else (void)0

bool Scale_parse(Scale* scale, char* str, Read_state* state)
{
    assert(scale != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Scale* new_scale = Scale_init(&(Scale){ .note_count = 0 },
                                  SCALE_DEFAULT_REF_PITCH,
                                  SCALE_DEFAULT_OCTAVE_RATIO);
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (!state->error)
        {
            Scale_copy(scale, new_scale);
            return true;
        }
        Read_state_clear_error(state);
        
        bool expect_key = true;
        while (expect_key)
        {
            char key[256] = { '\0' };
            str = read_string(str, key, 256, state);
            str = read_const_char(str, ':', state);
            if (state->error)
            {
                return false;
            }
            if (strcmp(key, "ref_note") == 0)
            {
                int64_t num = 0;
                str = read_int(str, &num, state);
                if (state->error)
                {
                    return false;
                }
                if (num < 0 || num >= KQT_SCALE_NOTES)
                {
                    Read_state_set_error(state,
                             "Invalid reference note number: %" PRId64, num);
                    return false;
                }
                new_scale->ref_note = num;
            }
            else if (strcmp(key, "ref_pitch") == 0)
            {
                double num = 0;
                str = read_double(str, &num, state);
                if (state->error)
                {
                    return false;
                }
                if (num <= 0 || !isfinite(num))
                {
                    Read_state_set_error(state,
                             "Invalid reference pitch: %f", num);
                    return false;
                }
                new_scale->ref_pitch = num;
            }
            else if (strcmp(key, "octave_ratio") == 0)
            {
                Real* ratio = Real_init(REAL_AUTO);
                double cents = NAN;
                read_and_validate_tuning(str, ratio, cents, state);
                if (!isnan(cents))
                {
                    Scale_set_octave_ratio_cents(new_scale, cents);
                }
                else
                {
                    Scale_set_octave_ratio(new_scale, ratio);
                }
            }
            else if (strcmp(key, "note_mods") == 0
                     || strcmp(key, "notes") == 0)
            {
                int count = 0;
                str = read_const_char(str, '[', state);
                if (state->error)
                {
                    return false;
                }
                str = read_const_char(str, ']', state);
                if (state->error)
                {
                    bool notes = strcmp(key, "notes") == 0;
                    Read_state_clear_error(state);
                    bool expect_val = true;
                    while (expect_val)
                    {
                        Real* ratio = Real_init(REAL_AUTO);
                        double cents = NAN;
                        read_and_validate_tuning(str, ratio, cents, state);
                        if (!isnan(cents))
                        {
                            if (notes)
                            {
                                Scale_set_note_cents(new_scale, count, cents);
                            }
                            else
                            {
                                Scale_set_note_mod_cents(new_scale, count, cents);
                            }
                        }
                        else
                        {
                            if (notes)
                            {
                                Scale_set_note(new_scale, count, ratio);
                            }
                            else
                            {
                                Scale_set_note_mod(new_scale, count, ratio);
                            }
                        }
                        ++count;
                        if (notes && count >= KQT_SCALE_NOTES)
                        {
                            break;
                        }
                        else if (!notes && count >= KQT_SCALE_NOTE_MODS)
                        {
                            break;
                        }
                        check_next(str, state, expect_val);
                    }
                    str = read_const_char(str, ']', state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (notes)
                    {
                        for (int i = count; i < KQT_SCALE_NOTES; ++i)
                        {
                            if (!NOTE_EXISTS(new_scale, i))
                            {
                                break;
                            }
                            NOTE_CLEAR(new_scale, i);
                        }
                    }
                }
            }
            else
            {
                Read_state_set_error(state,
                         "Unrecognised key in scale: %s", key);
                return false;
            }
            check_next(str, state, expect_key);
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            return false;
        }
        if (new_scale->ref_note >= new_scale->note_count)
        {
            Read_state_set_error(state,
                     "Reference note doesn't exist: %d\n", new_scale->ref_note);
            return false;
        }
    }
    Scale_copy(scale, new_scale);
    return true;
}

#undef read_and_validate_tuning


bool Scale_read(Scale* scale, File_tree* tree, Read_state* state)
{
    assert(scale != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    Scale_clear(scale);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Scale is not a directory");
        return false;
    }
    char* name = File_tree_get_name(tree);
    if (strncmp(name, MAGIC_ID, strlen(MAGIC_ID)) != 0)
    {
        Read_state_set_error(state, "Directory is not a Kunquat file");
        return false;
    }
    if (name[strlen(MAGIC_ID)] != 's')
    {
        Read_state_set_error(state, "Directory is not a scale file");
        return false;
    }
    const char* version = "00";
    if (strcmp(name + strlen(MAGIC_ID) + 1, version) != 0)
    {
        Read_state_set_error(state, "Unsupported scale version");
        return false;
    }
    File_tree* scale_tree = File_tree_get_child(tree, "p_scale.json");
    if (scale_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(scale_tree));
        if (File_tree_is_dir(scale_tree))
        {
            Read_state_set_error(state,
                     "Scale specification is a directory");
            return false;
        }
        char* str = File_tree_get_data(scale_tree);
        if (!Scale_parse(scale, str, state))
        {
            return false;
        }
    }
    return true;
}


void Scale_clear(Scale* scale)
{
    assert(scale != NULL);
    for (int i = 0; i < KQT_SCALE_NOTE_MODS; ++i)
    {
        NOTE_MOD_CLEAR(scale, i);
    }
    for (int i = 0; i < KQT_SCALE_NOTES; ++i)
    {
        NOTE_CLEAR(scale, i);
    }
    scale->note_count = 0;
    scale->ref_note = 0;
    scale->ref_note_retuned = 0;
    return;
}


int Scale_get_note_count(Scale* scale)
{
    assert(scale != NULL);
    return scale->note_count;
}


int Scale_get_note_mod_count(Scale* scale)
{
    assert(scale != NULL);
    int count = 0;
    while (count < KQT_SCALE_NOTE_MODS && NOTE_MOD_EXISTS(scale, count))
    {
        ++count;
    }
    return count;
}


bool Scale_set_ref_note(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (index >= scale->note_count)
    {
        return false;
    }
    scale->ref_note = index;
    return true;
}


int Scale_get_ref_note(Scale* scale)
{
    assert(scale != NULL);
    return scale->ref_note;
}


int Scale_get_cur_ref_note(Scale* scale)
{
    assert(scale != NULL);
    return scale->ref_note_retuned;
}


void Scale_set_ref_pitch(Scale* scale, pitch_t ref_pitch)
{
    assert(scale != NULL);
    assert(ref_pitch > 0);
    scale->ref_pitch = ref_pitch;
}


pitch_t Scale_get_ref_pitch(Scale* scale)
{
    assert(scale != NULL);
    return scale->ref_pitch;
}


void Scale_set_octave_ratio(Scale* scale, Real* octave_ratio)
{
    assert(scale != NULL);
    assert(octave_ratio != NULL);
    assert( Real_cmp(octave_ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    Real_copy(&(scale->octave_ratio), octave_ratio);
    Real_init_as_frac(&(scale->oct_factors[KQT_SCALE_OCTAVES / 2]), 1, 1);
    for (int i = KQT_SCALE_MIDDLE_OCTAVE_UNBIASED - 1; i >= 0; --i)
    {
        Real_div(&(scale->oct_factors[i]),
                 &(scale->oct_factors[i + 1]),
                 &(scale->octave_ratio));
    }
    for (int i = KQT_SCALE_MIDDLE_OCTAVE_UNBIASED + 1; i < KQT_SCALE_OCTAVES; ++i)
    {
        Real_mul(&(scale->oct_factors[i]),
                 &(scale->oct_factors[i - 1]),
                 &(scale->octave_ratio));
    }
    scale->oct_ratio_cents = NAN;
    return;
}


Real* Scale_get_octave_ratio(Scale* scale)
{
    assert(scale != NULL);
    return &(scale->octave_ratio);
}


void Scale_set_octave_ratio_cents(Scale* scale, double cents)
{
    assert(scale != NULL);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    Scale_set_octave_ratio(scale, ratio);
    scale->oct_ratio_cents = cents;
    return;
}


double Scale_get_octave_ratio_cents(Scale* scale)
{
    assert(scale != NULL);
    return scale->oct_ratio_cents;
}


int Scale_set_note(Scale* scale,
        int index,
        Real* ratio)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    while (index > 0 && !NOTE_EXISTS(scale, index - 1))
    {
        assert(!NOTE_EXISTS(scale, index));
        --index;
    }
    if (!NOTE_EXISTS(scale, index))
    {
        if (scale->note_count < KQT_SCALE_NOTES)
        {
            ++(scale->note_count);
        }
    }
    NOTE_CLEAR(scale, index);
    Real_copy(&(scale->notes[index].ratio), ratio);
    Real_copy(&(scale->notes[index].ratio_retuned), ratio);
    return index;
}


int Scale_set_note_cents(Scale* scale,
        int index,
        double cents)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Scale_set_note(scale, index, ratio);
    assert(actual_index >= 0);
    assert(actual_index < KQT_SCALE_NOTES);
    scale->notes[actual_index].cents = cents;
    return actual_index;
}


int Scale_ins_note(Scale* scale,
        int index,
        Real* ratio)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    if (!NOTE_EXISTS(scale, index))
    {
        return Scale_set_note(scale, index, ratio);
    }
    if (scale->note_count < KQT_SCALE_NOTES)
    {
        ++(scale->note_count);
    }
    int i = MIN(scale->note_count, KQT_SCALE_NOTES - 1);
/*    for (i = index; (i < KQT_SCALE_NOTES - 1) && NOTE_EXISTS(scale, i); ++i)
        ; */
    for (; i > index; --i)
    {
        scale->notes[i].cents = scale->notes[i - 1].cents;
        Real_copy(&(scale->notes[i].ratio), &(scale->notes[i - 1].ratio));
        Real_copy(&(scale->notes[i].ratio_retuned),
                &(scale->notes[i - 1].ratio_retuned));
    }
    assert(NOTE_EXISTS(scale, MIN(scale->note_count, KQT_SCALE_NOTES - 1))
            == (scale->note_count == KQT_SCALE_NOTES));
    NOTE_CLEAR(scale, index);
    Real_copy(&(scale->notes[index].ratio), ratio);
    Real_copy(&(scale->notes[index].ratio_retuned), ratio);
    return index;
}


int Scale_ins_note_cents(Scale* scale,
        int index,
        double cents)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Scale_ins_note(scale, index, ratio);
    assert(actual_index >= 0);
    assert(actual_index < KQT_SCALE_NOTES);
    scale->notes[actual_index].cents = cents;
    return actual_index;
}


void Scale_del_note(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (!NOTE_EXISTS(scale, index))
    {
        return;
    }
    if (scale->note_count > 0)
    {
        --(scale->note_count);
        if ((scale->ref_note >= scale->note_count)
                && (scale->ref_note > 0))
        {
            --(scale->ref_note);
        }
        if ((scale->ref_note_retuned >= scale->note_count)
                && (scale->ref_note_retuned > 0))
        {
            --(scale->ref_note_retuned);
        }
    }
    int i = 0;
    for (i = index; (i < KQT_SCALE_NOTES - 1) && NOTE_EXISTS(scale, i + 1); ++i)
    {
        scale->notes[i].cents = scale->notes[i + 1].cents;
        Real_copy(&(scale->notes[i].ratio), &(scale->notes[i + 1].ratio));
        Real_copy(&(scale->notes[i].ratio_retuned),
                  &(scale->notes[i + 1].ratio_retuned));
    }
    NOTE_CLEAR(scale, i);
    return;
}


int Scale_move_note(Scale* scale, int index, int new_index)
{
    // TODO: optimise?
    int actual_index = -1;
    double tmpcents;
    Real tmpratio;
    Real tmpratio_retuned;
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(new_index >= 0);
    assert(new_index < KQT_SCALE_NOTES);
    if (index == new_index)
    {
        return index;
    }
    if (!NOTE_EXISTS(scale, index))
    {
        return index;
    }
    tmpcents = scale->notes[index].cents;
    Real_copy(&(tmpratio), &(scale->notes[index].ratio));
    Real_copy(&(tmpratio_retuned), &(scale->notes[index].ratio_retuned));
    Scale_del_note(scale, index);
    actual_index = Scale_ins_note(scale, new_index, &tmpratio);
    if (NOTE_EXISTS(scale, new_index))
    {
        Real_copy(&(scale->notes[new_index].ratio_retuned), &tmpratio_retuned);
    }
    else
    {
        assert(scale->note_count > 0);
        Real_copy(&(scale->notes[scale->note_count - 1].ratio_retuned),
                  &tmpratio_retuned);
    }
    assert(actual_index >= 0);
    scale->notes[actual_index].cents = tmpcents;
    return actual_index;
}


Real* Scale_get_note_ratio(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (!NOTE_EXISTS(scale, index))
    {
        return NULL;
    }
    return &(scale->notes[index].ratio);
}


Real* Scale_get_cur_note_ratio(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (!NOTE_EXISTS(scale, index))
    {
        return NULL;
    }
    return &scale->notes[index].ratio_retuned;
}


double Scale_get_note_cents(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (!NOTE_EXISTS(scale, index))
    {
        return NAN;
    }
    return scale->notes[index].cents;
}


double Scale_get_cur_note_cents(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    if (!NOTE_EXISTS(scale, index))
    {
        return NAN;
    }
    double val = Real_get_double(&scale->notes[index].ratio_retuned);
    return log2(val) * 1200;
}


int Scale_set_note_mod(Scale* scale,
        int index,
        Real* ratio)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    while (index > 0 && !NOTE_MOD_EXISTS(scale, index - 1))
    {
        assert(!NOTE_MOD_EXISTS(scale, index));
        --index;
    }
    NOTE_MOD_CLEAR(scale, index);
    Real_copy(&(scale->note_mods[index].ratio), ratio);
    return index;
}


int Scale_set_note_mod_cents(Scale* scale,
        int index,
        double cents)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Scale_set_note_mod(scale, index, ratio);
    assert(actual_index >= 0);
    assert(actual_index < KQT_SCALE_NOTE_MODS);
    scale->note_mods[actual_index].cents = cents;
    return actual_index;
}


int Scale_ins_note_mod(Scale* scale,
        int index,
        Real* ratio)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    if (!NOTE_MOD_EXISTS(scale, index))
    {
        return Scale_set_note_mod(scale, index, ratio);
    }
    int i = 0;
    for (i = index; (i < KQT_SCALE_NOTE_MODS - 1)
            && NOTE_MOD_EXISTS(scale, i); ++i)
        ;
    for (; i > index; --i)
    {
        scale->note_mods[i].cents = scale->note_mods[i - 1].cents;
        Real_copy(&(scale->note_mods[i].ratio),
                  &(scale->note_mods[i - 1].ratio));
    }
    scale->note_mods[index].cents = NAN;
    Real_copy(&(scale->note_mods[index].ratio), ratio);
    return index;
}


int Scale_ins_note_mod_cents(Scale* scale,
        int index,
        double cents)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Scale_ins_note_mod(scale, index, ratio);
    assert(actual_index >= 0);
    assert(actual_index < KQT_SCALE_NOTE_MODS);
    scale->note_mods[actual_index].cents = cents;
    return actual_index;
}


void Scale_del_note_mod(Scale* scale, int index)
{
    int i = 0;
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(scale, index))
    {
        return;
    }
    for (i = index; (i < KQT_SCALE_NOTE_MODS - 1)
            && NOTE_MOD_EXISTS(scale, i + 1); ++i)
    {
        scale->note_mods[i].cents = scale->note_mods[i + 1].cents;
        Real_copy(&(scale->note_mods[i].ratio),
                  &(scale->note_mods[i + 1].ratio));
    }
    NOTE_MOD_CLEAR(scale, i);
    return;
}


int Scale_move_note_mod(Scale* scale, int index, int new_index)
{
    //* TODO: optimise?
    double tmpcents;
    Real tmpratio;
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    assert(new_index >= 0);
    assert(new_index < KQT_SCALE_NOTE_MODS);
    if (index == new_index)
    {
        return index;
    }
    if (!NOTE_MOD_EXISTS(scale, index))
    {
        return index;
    }
    tmpcents = scale->note_mods[index].cents;
    Real_copy(&(tmpratio), &(scale->note_mods[index].ratio));
    Scale_del_note_mod(scale, index);
    int ret = Scale_ins_note_mod(scale, new_index, &tmpratio);
    assert(ret >= 0);
    scale->note_mods[ret].cents = tmpcents;
    return ret;
}


Real* Scale_get_note_mod_ratio(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(scale, index))
    {
        return NULL;
    }
    return &(scale->note_mods[index].ratio);
}


double Scale_get_note_mod_cents(Scale* scale, int index)
{
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(scale, index))
    {
        return NAN;
    }
    return scale->note_mods[index].cents;
}


pitch_t Scale_get_pitch(Scale* scale,
        int index,
        int mod,
        int octave)
{
    octave -= KQT_SCALE_OCTAVE_BIAS;
    Real final_ratio;
    assert(scale != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALE_NOTES);
    assert(mod < KQT_SCALE_NOTE_MODS);
    assert(octave >= 0);
    assert(octave < KQT_SCALE_OCTAVES);
    if (!NOTE_EXISTS(scale, index))
    {
        return -1;
    }
    Real_copy(&final_ratio,
              &(scale->notes[index].ratio_retuned));
    if (mod >= 0 && NOTE_MOD_EXISTS(scale, mod))
    {
        Real_mul(&final_ratio,
                 &(scale->notes[index].ratio_retuned),
                 &(scale->note_mods[mod].ratio));
    }
    Real_mul(&final_ratio,
             &final_ratio,
             &(scale->oct_factors[octave]));
    return (pitch_t)Real_mul_float(&final_ratio, (double)(scale->ref_pitch));
}


void Scale_retune(Scale* scale, int new_ref, int fixed_point)
{
    assert(scale != NULL);
    assert(new_ref < KQT_SCALE_NOTES);
    assert(fixed_point >= 0);
    assert(fixed_point < KQT_SCALE_NOTES);
    if (new_ref < 0)
    {
        // reset to original
        scale->ref_note_retuned = scale->ref_note;
        for (int i = 0; (i < KQT_SCALE_NOTES) && NOTE_EXISTS(scale, i); ++i)
        {
            Real_copy(&(scale->notes[i].ratio_retuned),
                      &(scale->notes[i].ratio));
        }
        return;
    }
    if ((new_ref == scale->ref_note_retuned)
            || !NOTE_EXISTS(scale, new_ref))
    {
        return;
    }
    if (!NOTE_EXISTS(scale, fixed_point))
    {
        fixed_point = scale->ref_note_retuned; // TODO: any better way?
    }
    // retune new_ref
    int fixed_new_order = fixed_point - new_ref;
    if (fixed_new_order < 0)
    {
        fixed_new_order += scale->note_count;
    }
    assert(fixed_new_order >= 0);
    int fixed_counterpart = (scale->ref_note_retuned + fixed_new_order)
            % scale->note_count;
    Real fixed_to_new_ref_ratio;
    Real_div(&fixed_to_new_ref_ratio,
             &(scale->notes[fixed_counterpart].ratio_retuned),
             &(scale->notes[scale->ref_note_retuned].ratio_retuned));
    if ((fixed_counterpart > scale->ref_note_retuned)
            && (fixed_point < new_ref))
    {
        Real_div(&fixed_to_new_ref_ratio,
                 &fixed_to_new_ref_ratio,
                 &(scale->octave_ratio));
    }
    else if ((fixed_counterpart < scale->ref_note_retuned)
            && (fixed_point > new_ref))
    {
        Real_mul(&fixed_to_new_ref_ratio,
                 &fixed_to_new_ref_ratio,
                 &(scale->octave_ratio));
    }
    static Real new_notes[KQT_SCALE_NOTES];
    Real_div(&(new_notes[new_ref]),
             &(scale->notes[fixed_point].ratio_retuned),
             &fixed_to_new_ref_ratio);
    /* new_ref is now retuned -- retune other notes excluding fixed_point */
    for (int i = 1; i < scale->note_count; ++i)
    {
        Real to_ref_ratio;
        int cur_from_old_ref = (scale->ref_note_retuned + i) % scale->note_count;
        int cur_from_new_ref = (new_ref + i) % scale->note_count;
        if (cur_from_new_ref == fixed_point)
        {
            Real_copy(&(new_notes[fixed_point]),
                      &(scale->notes[fixed_point].ratio_retuned));
            continue;
        }
        Real_div(&to_ref_ratio,
                 &(scale->notes[cur_from_old_ref].ratio_retuned),
                 &(scale->notes[scale->ref_note_retuned].ratio_retuned));
        if ((cur_from_new_ref > new_ref)
                && (cur_from_old_ref < scale->ref_note_retuned))
        {
            Real_mul(&to_ref_ratio,
                     &to_ref_ratio,
                     &(scale->octave_ratio));
        }
        else if ((cur_from_new_ref < new_ref)
                && (cur_from_old_ref > scale->ref_note_retuned))
        {
            Real_div(&to_ref_ratio,
                     &to_ref_ratio,
                     &(scale->octave_ratio));
        }
        Real_mul(&(new_notes[cur_from_new_ref]),
                 &to_ref_ratio,
                 &(new_notes[new_ref]));
    }
    /* update */
    scale->ref_note_retuned = new_ref;
    for (int i = 0; i < scale->note_count; ++i)
    {
        Real_copy(&(scale->notes[i].ratio_retuned),
                  &(new_notes[i]));
    }
    return;
}


Real* Scale_drift(Scale* scale, Real* drift)
{
    assert(scale != NULL);
    assert(drift != NULL);
    return Real_div(drift, &scale->notes[scale->ref_note].ratio_retuned,
                    &scale->notes[scale->ref_note].ratio);
}


void del_Scale(Scale* scale)
{
    assert(scale != NULL);
    xfree(scale);
    return;
}


