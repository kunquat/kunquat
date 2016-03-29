

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/Tuning_table.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TUNING_TABLE_DEFAULT_REF_PITCH 300
#define TUNING_TABLE_DEFAULT_OCTAVE_WIDTH 1200


typedef struct pitch_index
{
    double cents;
    int note;
    int octave;
} pitch_index;


static int pitch_index_cmp(const pitch_index* pi1, const pitch_index* pi2)
{
    assert(pi1 != NULL);
    assert(pi2 != NULL);

    if (pi1->cents < pi2->cents)
        return -1;
    else if (pi1->cents > pi2->cents)
        return 1;
    return 0;
}


static bool Tuning_table_build_pitch_map(Tuning_table* tt)
{
    assert(tt != NULL);

    AAtree* pitch_map = new_AAtree(
            (int (*)(const void*, const void*))pitch_index_cmp,
            memory_free);
    if (pitch_map == NULL)
        return false;

    for (int octave = 0; octave < KQT_TUNING_TABLE_OCTAVES; ++octave)
    {
        for (int note = 0; note < tt->note_count; ++note)
        {
            pitch_index* pi = memory_alloc_item(pitch_index);
            if (pi == NULL)
            {
                del_AAtree(pitch_map);
                return false;
            }

            pi->cents =
                tt->ref_pitch + tt->note_offsets[note] + tt->octave_offsets[octave];
            pi->note = note;
            pi->octave = octave;

            if (!AAtree_ins(pitch_map, pi))
            {
                del_AAtree(pitch_map);
                return false;
            }
        }
    }

    if (tt->pitch_map != NULL)
        del_AAtree(tt->pitch_map);

    tt->pitch_map = pitch_map;

    return true;
}


/**
 * Set a new note in the Tuning table using cents.
 *
 * Any existing note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
static void Tuning_table_set_note_cents(Tuning_table* tt, int index, double cents);


void Tuning_table_set_octave_width(Tuning_table* tt, double octave_width);


/*
#define NOTE_EXISTS(tt, index) (Real_get_numerator(&(tt)->notes[(index)].ratio) >= 0)

#define NOTE_CLEAR(tt, index)                                          \
    if (true)                                                          \
    {                                                                  \
        (tt)->notes[(index)].cents = NAN;                              \
        Real_init_as_frac(&(tt)->notes[(index)].ratio, -1, 1);         \
        Real_init_as_frac(&(tt)->notes[(index)].ratio_retuned, -1, 1); \
    } else (void)0
// */


Tuning_table* new_Tuning_table(double ref_pitch, double octave_width)
{
    assert(ref_pitch > 0);
    assert(isfinite(octave_width));
    assert(octave_width > 0);

    Tuning_table* tt = memory_alloc_item(Tuning_table);
    if (tt == NULL)
        return NULL;

    tt->pitch_map = NULL;
    tt->note_count = 0;
    tt->ref_note = 0;
    tt->ref_pitch = ref_pitch;
    tt->global_offset = 0;
    Tuning_table_set_octave_width(tt, octave_width);

    if (!Tuning_table_build_pitch_map(tt))
    {
        memory_free(tt);
        return NULL;
    }

    return tt;
}


static bool Streader_read_tuning(Streader* sr, double* cents)
{
    assert(sr != NULL);
    assert(cents != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (Streader_try_match_char(sr, '['))
    {
        int64_t num = 0;
        int64_t den = 0;
        if (!Streader_readf(sr, "%i,%i]", &num, &den))
            return false;

        if (num <= 0)
        {
            Streader_set_error(sr, "Numerator must be positive");
            return false;
        }
        if (den <= 0)
        {
            Streader_set_error(sr, "Denominator must be positive");
            return false;
        }

        *cents = log2((double)num / (double)den) * 1200;
    }
    else
    {
        if (!Streader_read_float(sr, cents))
            return false;

        if (!isfinite(*cents) || *cents <= 0)
        {
            Streader_set_error(sr, "Cents value must be finite and positive");
            return false;
        }
    }

    return true;
}


static bool read_note(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);

    if (index >= KQT_TUNING_TABLE_NOTES)
    {
        Streader_set_error(sr, "Too many notes in the tuning table");
        return false;
    }

    Tuning_table* tt = userdata;

    double cents = NAN;
    if (!Streader_read_tuning(sr, &cents))
        return false;

    Tuning_table_set_note_cents(tt, index, cents);

    return true;
}

static bool read_tuning_table_item(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    Tuning_table* tt = userdata;

    if (string_eq(key, "ref_note"))
    {
        int64_t num = 0;
        if (!Streader_read_int(sr, &num))
            return false;

        if (num < 0 || num >= KQT_TUNING_TABLE_NOTES)
        {
            Streader_set_error(
                     sr, "Invalid reference note number: %" PRId64, num);
            return false;
        }

        tt->ref_note = num;
    }
    else if (string_eq(key, "ref_pitch"))
    {
        double num = NAN;
        if (!Streader_read_float(sr, &num))
            return false;

        if (!isfinite(num))
        {
            Streader_set_error(sr, "Invalid reference pitch: %f", num);
            return false;
        }

        tt->ref_pitch = num;
    }
    else if (string_eq(key, "pitch_offset"))
    {
        double cents = NAN;
        if (!Streader_read_float(sr, &cents))
            return false;

        if (!isfinite(cents))
        {
            Streader_set_error(sr, "Pitch offset is not finite");
            return false;
        }

        tt->global_offset = cents;
    }
    else if (string_eq(key, "octave_width"))
    {
        double cents = NAN;
        if (!Streader_read_tuning(sr, &cents))
            return false;

        Tuning_table_set_octave_width(tt, cents);
    }
    else if (string_eq(key, "notes"))
    {
        for (int i = 0; i < KQT_TUNING_TABLE_NOTES; ++i)
            tt->note_offsets[i] = NAN;

        if (!Streader_read_list(sr, read_note, tt))
            return false;
    }
    else
    {
        Streader_set_error(sr, "Unrecognised key in tuning table: %s", key);
        return false;
    }

    return true;
}


Tuning_table* new_Tuning_table_from_string(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Tuning_table* tt = new_Tuning_table(
            TUNING_TABLE_DEFAULT_REF_PITCH, TUNING_TABLE_DEFAULT_OCTAVE_WIDTH);
    if (tt == NULL)
    {
        Streader_set_memory_error(
                sr, "Couldn't allocate memory for tuning table");
        return NULL;
    }

    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_tuning_table_item, tt))
        {
            del_Tuning_table(tt);
            return NULL;
        }

        if (tt->ref_note >= tt->note_count)
        {
            Streader_set_error(sr,
                     "Reference note doesn't exist: %d", tt->ref_note);
            del_Tuning_table(tt);
            return NULL;
        }
    }

    if (!Tuning_table_build_pitch_map(tt))
    {
        Streader_set_memory_error(sr, "Couldn't allocate memory for tuning table");
        del_Tuning_table(tt);
        return NULL;
    }

    return tt;
}


int Tuning_table_get_note_count(const Tuning_table* tt)
{
    assert(tt != NULL);
    return tt->note_count;
}


int Tuning_table_get_ref_note(const Tuning_table* tt)
{
    assert(tt != NULL);
    return tt->ref_note;
}


double Tuning_table_get_ref_pitch(const Tuning_table* tt)
{
    assert(tt != NULL);
    return tt->ref_pitch;
}


double Tuning_table_get_global_offset(const Tuning_table* tt)
{
    assert(tt != NULL);
    return tt->global_offset;
}


void Tuning_table_set_octave_width(Tuning_table* tt, double octave_width)
{
    assert(tt != NULL);
    assert(isfinite(octave_width));
    assert(octave_width > 0);

    tt->octave_width = octave_width;
    for (int i = 0; i < KQT_TUNING_TABLE_OCTAVES; ++i)
    {
        const int rel_octave = i - KQT_TUNING_TABLE_MIDDLE_OCTAVE_UNBIASED;
        tt->octave_offsets[i] = rel_octave * octave_width;
    }

    return;
}


double Tuning_table_get_octave_width(const Tuning_table* tt)
{
    assert(tt != NULL);
    return tt->octave_width;
}


static void Tuning_table_set_note_cents(Tuning_table* tt, int index, double cents)
{
    assert(tt != NULL);
    assert(index >= 0);
    assert(index < KQT_TUNING_TABLE_NOTES);
    assert(isfinite(cents));

    tt->note_offsets[index] = cents;

    return;
}


double Tuning_table_get_pitch_offset(const Tuning_table* tt, int index)
{
    assert(tt != NULL);
    assert(index >= 0);
    assert(index < Tuning_table_get_note_count(tt));

    return tt->note_offsets[index];
}


#if 0
double Tuning_table_get_pitch(Tuning_table* tt, int index, int octave)
{
    octave -= KQT_TUNING_TABLE_OCTAVE_BIAS;
    Real final_ratio;
    assert(tt != NULL);
    assert(index >= 0);
    assert(index < KQT_TUNING_TABLE_NOTES);
    assert(octave >= 0);
    assert(octave < KQT_TUNING_TABLE_OCTAVES);

    if (!NOTE_EXISTS(tt, index))
        return -1;

    Real_copy(&final_ratio,
              &(tt->notes[index].ratio_retuned));
    Real_mul(&final_ratio,
             &final_ratio,
             &(tt->oct_factors[octave]));

    return tt->pitch_offset *
           Real_mul_float(&final_ratio, (double)(tt->ref_pitch));
}
#endif


int Tuning_table_get_nearest_note_index(const Tuning_table* tt, double cents)
{
    assert(tt != NULL);
    assert(tt->note_count > 0);
    assert(tt->pitch_map != NULL);
    assert(isfinite(cents));

    pitch_index* key = &(pitch_index){ .cents = cents };
    pitch_index* pi_upper = AAtree_get_at_least(tt->pitch_map, key);
    pitch_index* pi_lower = AAtree_get_at_most(tt->pitch_map, key);
    pitch_index* pi = NULL;

    assert(pi_upper != NULL || pi_lower != NULL);

    if (pi_lower == NULL)
        pi = pi_upper;
    else if (pi_upper == NULL)
        pi = pi_lower;
    else if (fabs(pi_upper->cents - cents) < fabs(pi_lower->cents - cents))
        pi = pi_upper;
    else
        pi = pi_lower;

    return pi->note;
}


#if 0
double Tuning_table_get_pitch_from_cents(Tuning_table* tt, double cents)
{
    assert(tt != NULL);
    assert(isfinite(cents));

    if (tt->pitch_map == NULL)
        return -1;

    pitch_index* key = &(pitch_index){ .cents = cents };
    pitch_index* pi_upper = AAtree_get_at_least(tt->pitch_map, key);
    pitch_index* pi_lower = AAtree_get_at_most(tt->pitch_map, key);
    pitch_index* pi = NULL;

    if (pi_upper == NULL && pi_lower == NULL)
        return -1;
    else if (pi_lower == NULL)
        pi = pi_upper;
    else if (pi_upper == NULL)
        pi = pi_lower;
    else if (fabs(pi_upper->cents - cents) < fabs(pi_lower->cents - cents))
        pi = pi_upper;
    else
        pi = pi_lower;

    double hertz = exp2(cents / 1200) * 440;
    Real* retune = Real_div(
            REAL_AUTO,
            &tt->notes[pi->note].ratio_retuned,
            &tt->notes[pi->note].ratio);

    return tt->pitch_offset * Real_mul_float(retune, hertz);
}
#endif


#if 0
void Tuning_table_retune(Tuning_table* tt, int new_ref, int fixed_point)
{
    assert(tt != NULL);
    assert(new_ref < KQT_TUNING_TABLE_NOTES);
    assert(fixed_point >= 0);
    assert(fixed_point < KQT_TUNING_TABLE_NOTES);

    if (new_ref < 0)
    {
        // reset to original
        tt->ref_note_retuned = tt->ref_note;
        for (int i = 0; (i < KQT_TUNING_TABLE_NOTES) && NOTE_EXISTS(tt, i); ++i)
        {
            Real_copy(&(tt->notes[i].ratio_retuned),
                      &(tt->notes[i].ratio));
        }

        return;
    }

    if ((new_ref == tt->ref_note_retuned)
            || !NOTE_EXISTS(tt, new_ref))
        return;

    if (!NOTE_EXISTS(tt, fixed_point))
        fixed_point = tt->ref_note_retuned; // TODO: any better way?

    // retune new_ref
    int fixed_new_order = fixed_point - new_ref;
    if (fixed_new_order < 0)
        fixed_new_order += tt->note_count;

    assert(fixed_new_order >= 0);

    int fixed_counterpart = (tt->ref_note_retuned + fixed_new_order)
            % tt->note_count;
    Real fixed_to_new_ref_ratio;
    Real_div(&fixed_to_new_ref_ratio,
             &(tt->notes[fixed_counterpart].ratio_retuned),
             &(tt->notes[tt->ref_note_retuned].ratio_retuned));
    if ((fixed_counterpart > tt->ref_note_retuned)
            && (fixed_point < new_ref))
    {
        Real_div(&fixed_to_new_ref_ratio,
                 &fixed_to_new_ref_ratio,
                 &(tt->octave_ratio));
    }
    else if ((fixed_counterpart < tt->ref_note_retuned)
            && (fixed_point > new_ref))
    {
        Real_mul(&fixed_to_new_ref_ratio,
                 &fixed_to_new_ref_ratio,
                 &(tt->octave_ratio));
    }

    static Real new_notes[KQT_TUNING_TABLE_NOTES];
    Real_div(&(new_notes[new_ref]),
             &(tt->notes[fixed_point].ratio_retuned),
             &fixed_to_new_ref_ratio);

    /* new_ref is now retuned -- retune other notes excluding fixed_point */
    for (int i = 1; i < tt->note_count; ++i)
    {
        Real to_ref_ratio;
        int cur_from_old_ref = (tt->ref_note_retuned + i) % tt->note_count;
        int cur_from_new_ref = (new_ref + i) % tt->note_count;
        if (cur_from_new_ref == fixed_point)
        {
            Real_copy(&new_notes[fixed_point],
                      &tt->notes[fixed_point].ratio_retuned);
            continue;
        }

        Real_div(&to_ref_ratio,
                 &tt->notes[cur_from_old_ref].ratio_retuned,
                 &tt->notes[tt->ref_note_retuned].ratio_retuned);
        if ((cur_from_new_ref > new_ref)
                && (cur_from_old_ref < tt->ref_note_retuned))
        {
            Real_mul(&to_ref_ratio,
                     &to_ref_ratio,
                     &tt->octave_ratio);
        }
        else if ((cur_from_new_ref < new_ref)
                && (cur_from_old_ref > tt->ref_note_retuned))
        {
            Real_div(&to_ref_ratio,
                     &to_ref_ratio,
                     &tt->octave_ratio);
        }

        Real_mul(&new_notes[cur_from_new_ref],
                 &to_ref_ratio,
                 &new_notes[new_ref]);
    }

    /* update */
    tt->ref_note_retuned = new_ref;
    for (int i = 0; i < tt->note_count; ++i)
    {
        Real_copy(&tt->notes[i].ratio_retuned,
                  &new_notes[i]);
    }

    return;
}
#endif


void del_Tuning_table(Tuning_table* tt)
{
    if (tt == NULL)
        return;

    del_AAtree(tt->pitch_map);
    memory_free(tt);

    return;
}


