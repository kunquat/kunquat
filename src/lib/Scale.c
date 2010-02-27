

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
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
#include <math_common.h>

#include <xmemory.h>


typedef struct pitch_index
{
    double cents;
    int note;
    int octave;
} pitch_index;


static int pitch_index_cmp(const pitch_index* pi1, const pitch_index* pi2);

static int pitch_index_cmp(const pitch_index* pi1, const pitch_index* pi2)
{
    assert(pi1 != NULL);
    assert(pi2 != NULL);
    if (pi1->cents < pi2->cents)
    {
        return -1;
    }
    else if (pi1->cents > pi2->cents)
    {
        return 1;
    }
    return 0;
}


static bool Scale_build_pitch_map(Scale* scale);

static bool Scale_build_pitch_map(Scale* scale)
{
    assert(scale != NULL);
    AAtree* pitch_map = new_AAtree((int (*)(const void*, const void*))pitch_index_cmp,
                                   free);
    if (pitch_map == NULL)
    {
        return false;
    }
    for (int octave = 0; octave < KQT_SCALE_OCTAVES; ++octave)
    {
        for (int note = 0; note < scale->note_count; ++note)
        {
            pitch_index* pi = xalloc(pitch_index);
            if (pi == NULL)
            {
                del_AAtree(pitch_map);
                return false;
            }
            Real* scaled_ratio = Real_mul(REAL_AUTO,
                                          &scale->notes[note].ratio,
                                          &scale->oct_factors[octave]);
            double hertz = Real_mul_float(scaled_ratio, scale->ref_pitch);
            pi->cents = log2(hertz / 440) * 1200;
            pi->note = note;
            pi->octave = octave;
            if (!AAtree_ins(pitch_map, pi))
            {
                del_AAtree(pitch_map);
                return false;
            }
        }
    }
    if (scale->pitch_map != NULL)
    {
        del_AAtree(scale->pitch_map);
    }
    scale->pitch_map = pitch_map;
    return true;
}


/**
 * Sets a new note at the Scale.
 * Any existing note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
static int Scale_set_note(Scale* scale,
                          int index,
                          Real* ratio);


/**
 * Sets a new note at the Scale using cents.
 * Any existing note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
static int Scale_set_note_cents(Scale* scale,
                                int index,
                                double cents);


#define NOTE_EXISTS(scale, index) (Real_get_numerator(&(scale)->notes[(index)].ratio) >= 0)

#define NOTE_CLEAR(scale, index)                                          \
    if (true)                                                             \
    {                                                                     \
        (scale)->notes[(index)].cents = NAN;                              \
        Real_init_as_frac(&(scale)->notes[(index)].ratio, -1, 1);         \
        Real_init_as_frac(&(scale)->notes[(index)].ratio_retuned, -1, 1); \
    } else (void)0


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
    scale->pitch_map = NULL;
    scale->note_count = 0;
    scale->ref_note = scale->ref_note_retuned = 0;
    scale->ref_pitch = ref_pitch;
    Scale_set_octave_ratio(scale, octave_ratio);
    scale->oct_ratio_cents = NAN;
    if (!Scale_build_pitch_map(scale))
    {
        xfree(scale);
        return NULL;
    }
    Scale_clear(scale);
    return scale;
}


#define read_and_validate_tuning(str, ratio, cents, state)                        \
    if (true)                                                                     \
    {                                                                             \
        (str) = read_tuning((str), (ratio), &(cents), (state));                   \
        if ((state)->error)                                                       \
        {                                                                         \
            del_Scale(scale);                                                     \
            return NULL;                                                          \
        }                                                                         \
        if (isnan((cents)))                                                       \
        {                                                                         \
            if ((Real_is_frac((ratio)) && Real_get_numerator((ratio)) <= 0)       \
                    || (!Real_is_frac((ratio)) && Real_get_double((ratio)) <= 0)) \
            {                                                                     \
                del_Scale(scale);                                                 \
                Read_state_set_error((state), "Ratio is not positive");           \
                return NULL;                                                      \
            }                                                                     \
        }                                                                         \
    } else (void)0

Scale* new_Scale_from_string(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Scale* scale = new_Scale(SCALE_DEFAULT_REF_PITCH,
                             SCALE_DEFAULT_OCTAVE_RATIO);
    if (scale == NULL)
    {
        Read_state_set_error(state, "Couldn't allocate memory");
        return NULL;
    }
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            del_Scale(scale);
            return NULL;
        }
        str = read_const_char(str, '}', state);
        if (!state->error)
        {
            return scale;
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
                del_Scale(scale);
                return NULL;
            }
            if (strcmp(key, "ref_note") == 0)
            {
                int64_t num = 0;
                str = read_int(str, &num, state);
                if (state->error)
                {
                    del_Scale(scale);
                    return NULL;
                }
                if (num < 0 || num >= KQT_SCALE_NOTES)
                {
                    del_Scale(scale);
                    Read_state_set_error(state,
                             "Invalid reference note number: %" PRId64, num);
                    return NULL;
                }
                scale->ref_note = num;
            }
            else if (strcmp(key, "ref_pitch") == 0)
            {
                double num = 0;
                str = read_double(str, &num, state);
                if (state->error)
                {
                    del_Scale(scale);
                    return NULL;
                }
                if (num <= 0 || !isfinite(num))
                {
                    del_Scale(scale);
                    Read_state_set_error(state,
                             "Invalid reference pitch: %f", num);
                    return NULL;
                }
                scale->ref_pitch = num;
            }
            else if (strcmp(key, "octave_ratio") == 0)
            {
                Real* ratio = Real_init(REAL_AUTO);
                double cents = NAN;
                read_and_validate_tuning(str, ratio, cents, state);
                if (!isnan(cents))
                {
                    Scale_set_octave_ratio_cents(scale, cents);
                }
                else
                {
                    Scale_set_octave_ratio(scale, ratio);
                }
            }
            else if (strcmp(key, "notes") == 0)
            {
                int count = 0;
                str = read_const_char(str, '[', state);
                if (state->error)
                {
                    del_Scale(scale);
                    return NULL;
                }
                str = read_const_char(str, ']', state);
                if (state->error)
                {
                    Read_state_clear_error(state);
                    bool expect_val = true;
                    while (expect_val)
                    {
                        Real* ratio = Real_init(REAL_AUTO);
                        double cents = NAN;
                        read_and_validate_tuning(str, ratio, cents, state);
                        if (!isnan(cents))
                        {
                            Scale_set_note_cents(scale, count, cents);
                        }
                        else
                        {
                            Scale_set_note(scale, count, ratio);
                        }
                        ++count;
                        if (count >= KQT_SCALE_NOTES)
                        {
                            break;
                        }
                        check_next(str, state, expect_val);
                    }
                    str = read_const_char(str, ']', state);
                    if (state->error)
                    {
                        del_Scale(scale);
                        return NULL;
                    }
                    for (int i = count; i < KQT_SCALE_NOTES; ++i)
                    {
                        if (!NOTE_EXISTS(scale, i))
                        {
                            break;
                        }
                        NOTE_CLEAR(scale, i);
                    }
                }
            }
            else
            {
                del_Scale(scale);
                Read_state_set_error(state,
                         "Unrecognised key in scale: %s", key);
                return NULL;
            }
            check_next(str, state, expect_key);
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            del_Scale(scale);
            return NULL;
        }
        if (scale->ref_note >= scale->note_count)
        {
            del_Scale(scale);
            Read_state_set_error(state,
                     "Reference note doesn't exist: %d", scale->ref_note);
            return NULL;
        }
    }
    if (!Scale_build_pitch_map(scale))
    {
        del_Scale(scale);
        Read_state_set_error(state, "Couldn't allocate memory");
        return NULL;
    }
    return scale;
}

#undef read_and_validate_tuning


void Scale_clear(Scale* scale)
{
    assert(scale != NULL);
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


static int Scale_set_note(Scale* scale,
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


static int Scale_set_note_cents(Scale* scale,
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
        index = Scale_set_note(scale, index, ratio);
    }
    else
    {
        if (scale->note_count >= KQT_SCALE_NOTES)
        {
            assert(scale->note_count == KQT_SCALE_NOTES);
            return -1;
        }
        ++(scale->note_count);
        int i = MIN(scale->note_count, KQT_SCALE_NOTES - 1);
        /*for (i = index; (i < KQT_SCALE_NOTES - 1) && NOTE_EXISTS(scale, i); ++i)
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
    }
    for (int octave = 0; octave < KQT_SCALE_OCTAVES; ++octave)
    {
        pitch_index* pi = xalloc(pitch_index);
        if (pi == NULL)
        {
            return -1;
        }
        Real* scaled_ratio = Real_mul(REAL_AUTO,
                                      &scale->notes[index].ratio,
                                      &scale->oct_factors[octave]);
        double hertz = Real_mul_float(scaled_ratio, scale->ref_pitch);
        pi->cents = log2(hertz / 440) * 1200;
        pi->note = index;
        pi->octave = octave;
        if (!AAtree_ins(scale->pitch_map, pi))
        {
            xfree(pi);
            return -1;
        }
    }
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
    if (actual_index < 0)
    {
        return -1;
    }
    assert(actual_index < KQT_SCALE_NOTES);
    scale->notes[actual_index].cents = cents;
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
    Real_mul(&final_ratio,
             &final_ratio,
             &(scale->oct_factors[octave]));
    return (pitch_t)Real_mul_float(&final_ratio, (double)(scale->ref_pitch));
}


pitch_t Scale_get_pitch_from_cents(Scale* scale, double cents)
{
    assert(scale != NULL);
    assert(isfinite(cents));
    if (scale->pitch_map == NULL)
    {
        return -1;
    }
    pitch_index* key = &(pitch_index){ .cents = cents };
    pitch_index* pi_upper = AAtree_get(scale->pitch_map, key);
    pitch_index* pi_lower = AAtree_get_at_most(scale->pitch_map, key);
    pitch_index* pi = NULL;
    if (pi_upper == NULL && pi_lower == NULL)
    {
        return -1;
    }
    else if (pi_lower == NULL)
    {
        pi = pi_upper;
    }
    else if (pi_upper == NULL)
    {
        pi = pi_lower;
    }
    else if (fabs(pi_upper->cents - cents) < fabs(pi_lower->cents - cents))
    {
        pi = pi_upper;
    }
    else
    {
        pi = pi_lower;
    }
    double hertz = exp2(cents / 1200) * 440;
    Real* retune = Real_div(REAL_AUTO, &scale->notes[pi->note].ratio_retuned,
                            &scale->notes[pi->note].ratio);
    return Real_mul_float(retune, hertz);
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
    if (scale->pitch_map != NULL)
    {
        del_AAtree(scale->pitch_map);
    }
    xfree(scale);
    return;
}


