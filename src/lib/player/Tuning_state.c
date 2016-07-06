

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Tuning_state.h>

#include <debug/assert.h>
#include <init/Tuning_table.h>
#include <kunquat/limits.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


Tuning_state* new_Tuning_state(void)
{
    Tuning_state* ts = memory_alloc_item(Tuning_state);
    if (ts == NULL)
        return NULL;

    Tuning_state_reset(ts, NULL);

    return ts;
}


bool Tuning_state_can_retune(const Tuning_state* ts)
{
    rassert(ts != NULL);
    return (ts->note_count > 0);
}


void Tuning_state_reset(Tuning_state* ts, const Tuning_table* table)
{
    rassert(ts != NULL);

    if (table == NULL)
    {
        ts->note_count = 0;
        ts->ref_note = 0;
        ts->fixed_point = 0;
        ts->global_offset = 0;
        ts->drift = 0;
        for (int i = 0; i < KQT_TUNING_TABLE_NOTES_MAX; ++i)
            ts->note_offsets[i] = 0;
    }
    else
    {
        ts->note_count = Tuning_table_get_note_count(table);
        ts->ref_note = Tuning_table_get_ref_note(table);
        ts->fixed_point = 0;
        ts->global_offset = Tuning_table_get_global_offset(table);
        ts->drift = 0;
        for (int i = 0; i < ts->note_count; ++i)
            ts->note_offsets[i] = Tuning_table_get_pitch_offset(table, i);
        for (int i = ts->note_count; i < KQT_TUNING_TABLE_NOTES_MAX; ++i)
            ts->note_offsets[i] = 0;
    }

    return;
}


void Tuning_state_set_global_offset(Tuning_state* ts, double offset)
{
    rassert(ts != NULL);
    rassert(isfinite(offset));

    ts->global_offset = offset;

    return;
}


void Tuning_state_set_fixed_pitch(
        Tuning_state* ts, const Tuning_table* table, double pitch)
{
    rassert(ts != NULL);
    rassert(table != NULL);
    rassert(isfinite(pitch));

    const int note_index = Tuning_table_get_nearest_note_index(table, pitch);
    ts->fixed_point = note_index;

    return;
}


void Tuning_state_retune(Tuning_state* ts, const Tuning_table* table, double new_ref)
{
    rassert(ts != NULL);
    rassert(table != NULL);
    rassert(isfinite(new_ref));

    const int note_count = ts->note_count;
    if (note_count == 0)
        return;

    const int new_ref_index = Tuning_table_get_nearest_note_index(table, new_ref);

    const int shift_amount = (note_count + new_ref_index - ts->ref_note) % note_count;

    const double octave_width = Tuning_table_get_octave_width(table);

    // Get our current intervals
    double intervals[KQT_TUNING_TABLE_NOTES_MAX] = { 0 };
    for (int i = 0; i < note_count - 1; ++i)
        intervals[i] = ts->note_offsets[i + 1] - ts->note_offsets[i];
    intervals[note_count - 1] =
        ts->note_offsets[0] - ts->note_offsets[note_count - 1] + octave_width;

    // Fill new pitch offsets surrounding fixed point
    for (int i = ts->fixed_point + 1; i < note_count; ++i)
    {
        const int si = (i - shift_amount + note_count) % note_count;
        const int prev_si = (si - 1 + note_count) % note_count;
        ts->note_offsets[i] = ts->note_offsets[i - 1] + intervals[prev_si];
    }
    for (int i = ts->fixed_point - 1; i >= 0; --i)
    {
        const int si = (i - shift_amount + note_count) % note_count;
        ts->note_offsets[i] = ts->note_offsets[i + 1] - intervals[si];
    }

    ts->ref_note = new_ref_index;

    ts->drift = ts->note_offsets[ts->ref_note] -
        Tuning_table_get_pitch_offset(table, ts->ref_note);

    return;
}


double Tuning_state_get_retuned_pitch(
        const Tuning_state* ts, const Tuning_table* table, double cents)
{
    rassert(ts != NULL);
    rassert(table != NULL);
    rassert(isfinite(cents));

    if (ts->note_count == 0)
        return cents;

    const int note_index = Tuning_table_get_nearest_note_index(table, cents);
    const double start_offset = Tuning_table_get_pitch_offset(table, note_index);

    const double rel_offset = ts->note_offsets[note_index] - start_offset;

    return cents + rel_offset + ts->global_offset;
}


bool Tuning_state_retune_with_source(
        Tuning_state* ts, const Tuning_table* table, const Tuning_table* source)
{
    rassert(ts != NULL);
    rassert(table != NULL);
    rassert(source != NULL);

    if (Tuning_table_get_note_count(table) != Tuning_table_get_note_count(source))
        return false;

    for (int i = 0; i < ts->note_count; ++i)
        ts->note_offsets[i] = Tuning_table_get_pitch_offset(source, i);

    return true;
}


double Tuning_state_get_estimated_drift(const Tuning_state* ts)
{
    rassert(ts != NULL);
    return ts->drift;
}


void del_Tuning_state(Tuning_state* ts)
{
    if (ts == NULL)
        return;

    memory_free(ts);

    return;
}


