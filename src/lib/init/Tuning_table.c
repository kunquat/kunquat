

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
            (AAtree_item_cmp*)pitch_index_cmp, (AAtree_item_destroy*)memory_free);
    if (pitch_map == NULL)
        return false;

    for (int octave = 0; octave < KQT_TUNING_TABLE_OCTAVES; ++octave)
    {
        for (int note = 0; note < tt->note_count; ++note)
        {
            pitch_index* pi = &(pitch_index){ .cents = 0 };
            pi->cents =
                tt->ref_pitch + tt->note_offsets[note] + tt->octave_offsets[octave];
            pi->note = note;
            pi->octave = octave;

            if (!AAtree_contains(pitch_map, pi))
            {
                pitch_index* pi_entry = memory_alloc_item(pitch_index);
                if (pi_entry == NULL)
                {
                    del_AAtree(pitch_map);
                    return false;
                }
                *pi_entry = *pi;

                if (!AAtree_ins(pitch_map, pi_entry))
                {
                    del_AAtree(pitch_map);
                    return false;
                }
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
 *                less than the current note count.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 */
static void Tuning_table_set_note_cents(Tuning_table* tt, int index, double cents);


void Tuning_table_set_octave_width(Tuning_table* tt, double octave_width);


static Tuning_table* new_Tuning_table(double ref_pitch, double octave_width)
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
    tt->center_octave = 4;
    Tuning_table_set_octave_width(tt, octave_width);

    for (int i = 0; i < KQT_TUNING_TABLE_NOTES_MAX; ++i)
        tt->note_offsets[i] = 0;

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

        if (!isfinite(*cents))
        {
            Streader_set_error(sr, "Cents value must be finite");
            return false;
        }
    }

    return true;
}


static bool read_note(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);

    if (index >= KQT_TUNING_TABLE_NOTES_MAX)
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

        if (num < 0 || num >= KQT_TUNING_TABLE_NOTES_MAX)
        {
            Streader_set_error(
                     sr, "Invalid reference note number: %" PRId64, num);
            return false;
        }

        tt->ref_note = (int)num;
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

        if (cents <= 0)
        {
            Streader_set_error(sr, "Octave width must be positive");
            return false;
        }

        Tuning_table_set_octave_width(tt, cents);
    }
    else if (string_eq(key, "center_octave"))
    {
        int64_t octave = -1;
        if (!Streader_read_int(sr, &octave))
            return false;

        if (octave < 0 || octave >= KQT_TUNING_TABLE_OCTAVES)
        {
            Streader_set_error(sr, "Invalid center octave: %" PRId64, octave);
            return false;
        }

        tt->center_octave = (int)octave;
        Tuning_table_set_octave_width(tt, tt->octave_width);
    }
    else if (string_eq(key, "notes"))
    {
        for (int i = 0; i < KQT_TUNING_TABLE_NOTES_MAX; ++i)
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
        const int rel_octave = i - tt->center_octave;
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
    assert(index < KQT_TUNING_TABLE_NOTES_MAX);
    assert(index <= tt->note_count);
    assert(isfinite(cents));

    tt->note_offsets[index] = cents;

    if (index == tt->note_count)
        tt->note_count = index + 1;

    return;
}


double Tuning_table_get_pitch_offset(const Tuning_table* tt, int index)
{
    assert(tt != NULL);
    assert(index >= 0);
    assert(index < Tuning_table_get_note_count(tt));

    return tt->note_offsets[index];
}


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


void del_Tuning_table(Tuning_table* tt)
{
    if (tt == NULL)
        return;

    del_AAtree(tt->pitch_map);
    memory_free(tt);

    return;
}


