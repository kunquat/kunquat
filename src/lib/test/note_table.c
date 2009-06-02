

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
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <stdbool.h>

#include <check.h>

#include <Real.h>
#include <Note_table.h>


Suite* Note_table_suite(void);


START_TEST (new)
{
    int i = 0;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);

    table = new_Note_table(DBL_MIN, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    fail_unless(table->ref_pitch == DBL_MIN,
            "new_Note_table() didn't set reference pitch correctly.");
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "new_Note_table() didn't set octave ratio correctly.");
    Real_init_as_frac(&octave_ratio, 1, 1);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) != 0,
            "new_Note_table() incorrectly copied the reference of octave ratio.");
    Real_init_as_frac(&octave_ratio, 2, 1);
    fail_unless(table->note_count == 0,
            "new_Note_table() didn't create an empty table.");
    fail_unless(table->ref_note == table->ref_note_retuned,
            "new_Note_table() didn't set retuned reference note correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real quotient;
        Real_div(&quotient, &(table->oct_factors[i]), &(table->oct_factors[i - 1]));
        fail_unless(Real_cmp(&quotient, &octave_ratio) == 0,
                "new_Note_table() didn't create oct_factors[] correctly.");
    }
    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real one;
        Real_init_as_frac(&one, -1, 1);
        fail_unless(Real_cmp(&(table->note_mods[i].ratio), &one) == 0,
                "new_Note_table() didn't initialise note modifiers correctly.");
    }
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real one;
        Real_init_as_frac(&one, -1, 1);
        fail_unless(Real_cmp(&(table->notes[i].ratio), &one) == 0,
                "new_Note_table() didn't initialise notes correctly.");
        fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &one) == 0,
                "new_Note_table() didn't initialise notes correctly.");
    }
    del_Note_table(table);

    Real_init_as_frac(&octave_ratio, 1, 2);
    table = new_Note_table(DBL_MAX, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    fail_unless(table->ref_pitch == DBL_MAX,
            "new_Note_table() didn't set reference pitch correctly.");
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "new_Note_table() didn't set octave ratio correctly.");
    fail_unless(table->note_count == 0,
            "new_Note_table() didn't create an empty table.");
    fail_unless(table->ref_note == table->ref_note_retuned,
            "new_Note_table() didn't set retuned reference note correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real quotient;
        Real_div(&quotient, &(table->oct_factors[i]), &(table->oct_factors[i - 1]));
        fail_unless(Real_cmp(&quotient, &octave_ratio) == 0,
                "new_Note_table() didn't create oct_factors[] correctly.");
    }
    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real one;
        Real_init_as_frac(&one, -1, 1);
        fail_unless(Real_cmp(&(table->note_mods[i].ratio), &one) == 0,
                "new_Note_table() didn't initialise note modifiers correctly.");
    }
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real one;
        Real_init_as_frac(&one, -1, 1);
        fail_unless(Real_cmp(&(table->notes[i].ratio), &one) == 0,
                "new_Note_table() didn't initialise notes correctly.");
        fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &one) == 0,
                "new_Note_table() didn't initialise notes correctly.");
    }
    del_Note_table(table);

    Real_init_as_double(&octave_ratio, DBL_MIN);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "new_Note_table() didn't set octave ratio correctly.");
    del_Note_table(table);

    Real_init_as_double(&octave_ratio, DBL_MAX);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "new_Note_table() didn't set octave ratio correctly.");
    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break1)
{
    Real octave_ratio;
    Real_init(&octave_ratio);
    new_Note_table(0, &octave_ratio);
}
END_TEST

START_TEST (new_break2)
{
    Real octave_ratio;
    Real_init(&octave_ratio);
    new_Note_table(-DBL_MIN, &octave_ratio);
}
END_TEST

START_TEST (new_break3)
{
    Real octave_ratio;
    Real_init(&octave_ratio);
    new_Note_table(-DBL_MAX, &octave_ratio);
}
END_TEST

START_TEST (new_break4)
{
    new_Note_table(528, NULL);
}
END_TEST

START_TEST (new_break5)
{
    Real octave_ratio;
    Real_init_as_frac(&octave_ratio, 0, 1);
    new_Note_table(528, &octave_ratio);
}
END_TEST

START_TEST (new_break6)
{
    Real octave_ratio;
    Real_init_as_frac(&octave_ratio, -1, INT64_MAX);
    new_Note_table(528, &octave_ratio);
}
END_TEST

START_TEST (new_break7)
{
    Real octave_ratio;
    Real_init_as_frac(&octave_ratio, INT64_MIN, 1);
    new_Note_table(528, &octave_ratio);
}
END_TEST
#endif


START_TEST (set_ref_pitch)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    Note_table_set_ref_pitch(table, DBL_MIN);
    fail_unless(table->ref_pitch == DBL_MIN,
            "Note_table_set_ref_pitch() didn't set reference pitch correctly.");
    Note_table_set_ref_pitch(table, DBL_MAX);
    fail_unless(table->ref_pitch == DBL_MAX,
            "Note_table_set_ref_pitch() didn't set reference pitch correctly.");
    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_ref_pitch_break1)
{
    Note_table_set_ref_pitch(NULL, 528);
}
END_TEST

START_TEST (set_ref_pitch_break2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    Note_table_set_ref_pitch(table, 0);
    del_Note_table(table);
}
END_TEST

START_TEST (set_ref_pitch_break3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    Note_table_set_ref_pitch(table, -DBL_MIN);
    del_Note_table(table);
}
END_TEST

START_TEST (set_ref_pitch_break4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    Note_table_set_ref_pitch(table, -DBL_MAX);
    del_Note_table(table);
}
END_TEST
#endif

START_TEST (get_ref_pitch)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    fail_unless(Note_table_get_ref_pitch(table) == 528,
            "Note_table_get_ref_pitch() didn't return the correct reference pitch.");
    Note_table_set_ref_pitch(table, DBL_MIN);
    fail_unless(Note_table_get_ref_pitch(table) == DBL_MIN,
            "Note_table_get_ref_pitch() didn't return the correct reference pitch.");
    Note_table_set_ref_pitch(table, DBL_MAX);
    fail_unless(Note_table_get_ref_pitch(table) == DBL_MAX,
            "Note_table_get_ref_pitch() didn't return the correct reference pitch.");
    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (get_ref_pitch_break)
{
    Note_table_get_ref_pitch(NULL);
}
END_TEST
#endif

START_TEST (set_octave_ratio)
{
    int i = 0;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    Real_init_as_frac(&octave_ratio, 2, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real quotient;
        Real_div(&quotient, &(table->oct_factors[i]), &(table->oct_factors[i - 1]));
        fail_unless(Real_cmp(&quotient, &octave_ratio) == 0,
                "Note_table_set_octave_ratio() didn't create oct_factors[] correctly.");
    }
    Real_init_as_frac(&octave_ratio, 1, 1);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) != 0,
            "Note_table_set_octave_ratio() incorrectly copied the reference of octave ratio.");

    Real_init_as_frac(&octave_ratio, 1, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        fail_unless(Real_cmp(&(table->oct_factors[i]), &(table->oct_factors[i - 1])) == 0,
                "Note_table_set_octave_ratio() didn't create oct_factors[] correctly.");
    }

    Real_init_as_frac(&octave_ratio, 3, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real quotient;
        Real_div(&quotient, &(table->oct_factors[i]), &(table->oct_factors[i - 1]));
        fail_unless(Real_cmp(&quotient, &octave_ratio) == 0,
                "Note_table_set_octave_ratio() didn't create oct_factors[] correctly.");
    }

    Real_init_as_frac(&octave_ratio, 1, 3);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");
    for (i = 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real quotient;
        Real_div(&quotient, &(table->oct_factors[i]), &(table->oct_factors[i - 1]));
        fail_unless(Real_cmp(&quotient, &octave_ratio) == 0,
                "Note_table_set_octave_ratio() didn't create oct_factors[] correctly.");
    }

    Real_init_as_double(&octave_ratio, DBL_MIN);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");

    Real_init_as_double(&octave_ratio, DBL_MAX);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, &(table->octave_ratio)) == 0,
            "Note_table_set_octave_ratio() didn't set octave ratio correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_octave_ratio_break1)
{
    Real octave_ratio;
    Real_init(&octave_ratio);
    Note_table_set_octave_ratio(NULL, &octave_ratio);
}
END_TEST

START_TEST (set_octave_ratio_break2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    
    Note_table_set_octave_ratio(table, NULL);

    del_Note_table(table);
}
END_TEST

START_TEST (set_octave_ratio_break3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    
    Real_init_as_frac(&octave_ratio, 0, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_octave_ratio_break4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    
    Real_init_as_frac(&octave_ratio, -1, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_octave_ratio_break5)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }
    
    Real_init_as_frac(&octave_ratio, INT64_MIN, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (get_octave_ratio)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 1, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    fail_unless(Real_cmp(&octave_ratio, Note_table_get_octave_ratio(table)) == 0,
            "Note_table_get_octave_ratio() didn't return the correct octave ratio.");
    Real_init_as_frac(&octave_ratio, INT64_MAX, 1);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, Note_table_get_octave_ratio(table)) == 0,
            "Note_table_get_octave_ratio() didn't return the correct octave ratio.");
    Real_init_as_double(&octave_ratio, DBL_MIN);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, Note_table_get_octave_ratio(table)) == 0,
            "Note_table_get_octave_ratio() didn't return the correct octave ratio.");
    Real_init_as_double(&octave_ratio, DBL_MAX);
    Note_table_set_octave_ratio(table, &octave_ratio);
    fail_unless(Real_cmp(&octave_ratio, Note_table_get_octave_ratio(table)) == 0,
            "Note_table_get_octave_ratio() didn't return the correct octave ratio.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (get_octave_ratio_break)
{
    Note_table_get_octave_ratio(NULL);
}
END_TEST
#endif

START_TEST (set_note)
{
    int i = 0;
    int actual_index = -1;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_set_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_set_note() didn't set the retuned note ratio correctly.");
    Real_init_as_frac(&note_ratio, 1, 1);
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) != 0,
            "Note_table_set_note() incorrectly copied the reference of the note ratio.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) != 0,
            "Note_table_set_note() incorrectly copied the reference of the retuned note ratio.");
    fail_unless(table->note_count == 1,
            "Note_table_set_note() didn't increment the note count correctly.");

    for (i = 1; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i, i + 1);
        actual_index = Note_table_set_note(table, NOTE_TABLE_NOTES - 1, &note_ratio);
        fail_unless(actual_index == i,
                "Note_table_set_note() didn't return the correct index.");
        fail_unless(table->note_count == i + 1,
                "Note_table_set_note() didn't increment the note count correctly.");
        if (i < NOTE_TABLE_NOTES - 1)
        {
            fail_if(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio), &note_ratio) == 0,
                    "Note_table_set_note() incorrectly set the note ratio at the end of table.");
            fail_if(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio_retuned), &note_ratio) == 0,
                    "Note_table_set_note() incorrectly set the retuned note ratio at the end of table.");
        }
    }
    for (i = 1; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i, i + 1);
        fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                "Note_table_set_note() didn't set the note ratio correctly.");
        fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                "Note_table_set_note() didn't set the retuned note ratio correctly.");
    }
    Real_init_as_frac(&note_ratio, 6, 5);
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    
    Real_init_as_frac(&note_ratio, 1, INT64_MAX);
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_set_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_set_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_frac(&note_ratio, INT64_MAX, 1);
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_set_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_set_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_double(&note_ratio, DBL_MIN);
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_set_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_set_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_double(&note_ratio, DBL_MAX);
    actual_index = Note_table_set_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_set_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_set_note() didn't set the retuned note ratio correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_note_break_table)
{
    Real note_ratio;
    Real_init(&note_ratio);
    Note_table_set_note(NULL, 0, &note_ratio);
}
END_TEST

START_TEST (set_note_break_index1)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, -1, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_index2)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, INT_MIN, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_index3)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, NOTE_TABLE_NOTES, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_index4)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, INT_MAX, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_ratio1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, 0, NULL);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_ratio2)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 0, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_ratio3)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, -1, INT64_MAX);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_break_ratio4)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, INT64_MIN, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (ins_note)
{
    int i = 0;
    int actual_index = -1;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    
    actual_index = Note_table_ins_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the retuned note ratio correctly.");
    Real_init_as_frac(&note_ratio, 1, 1);
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) != 0,
            "Note_table_ins_note() incorrectly copied the reference of the note ratio.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) != 0,
            "Note_table_ins_note() incorrectly copied the reference of the retuned note ratio.");
    fail_unless(table->note_count == 1,
            "Note_table_ins_note() didn't increment the note count correctly.");

    for (i = 0; i < NOTE_TABLE_NOTES - 1; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        actual_index = Note_table_ins_note(table, 0, &note_ratio);
        fail_unless(actual_index == 0,
                "Note_table_ins_note() didn't return the correct index.");
        fail_unless(table->note_count == i + 2,
                "Note_table_ins_note() didn't increment the note count correctly.");
/*      fprintf(stderr, "C should be at index %d, table->note_count = %d\n", i + 1, table->note_count); */
        if (i < NOTE_TABLE_NOTES - 2)
        {
            Real_init_as_frac(&note_ratio, -1, 1);
            fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio), &note_ratio) == 0,
                    "Note_table_ins_note() put garbage after the inserted notes.");
            fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio_retuned), &note_ratio) == 0,
                    "Note_table_ins_note() put garbage after the inserted notes.");
        }
        else
        {
            Real_init_as_frac(&note_ratio, 6, 5);
            fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio), &note_ratio) == 0,
                    "Note_table_ins_note() didn't shift notes forward correctly.");
            fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio_retuned), &note_ratio) == 0,
                    "Note_table_ins_note() didn't shift notes forward correctly.");
        }
    }
    for (i = 0; i < NOTE_TABLE_NOTES - 1; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        fail_unless(Real_cmp(&(table->notes[(NOTE_TABLE_NOTES - 2) - i].ratio), &note_ratio) == 0,
                "Note_table_ins_note() didn't shift notes forward correctly.");
        fail_unless(Real_cmp(&(table->notes[(NOTE_TABLE_NOTES - 2) - i].ratio_retuned), &note_ratio) == 0,
                "Note_table_ins_note() didn't shift notes forward correctly.");
    }
    Real_init_as_frac(&note_ratio, 4, 3);
    actual_index = Note_table_ins_note(table, 1, &note_ratio);
    fail_unless(actual_index == 1,
            "Note_table_ins_note() didn't return the correct index.");
    Real_init_as_frac(&note_ratio, 1, 2);
    fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't replace the last note correctly.");
    fail_unless(Real_cmp(&(table->notes[NOTE_TABLE_NOTES - 1].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't replace the last note correctly.");
    
    Real_init_as_frac(&note_ratio, 1, INT64_MAX);
    actual_index = Note_table_ins_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_frac(&note_ratio, INT64_MAX, 1);
    actual_index = Note_table_ins_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_double(&note_ratio, DBL_MIN);
    actual_index = Note_table_ins_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the retuned note ratio correctly.");
    
    Real_init_as_double(&note_ratio, DBL_MAX);
    actual_index = Note_table_ins_note(table, 0, &note_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->notes[0].ratio), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the note ratio correctly.");
    fail_unless(Real_cmp(&(table->notes[0].ratio_retuned), &note_ratio) == 0,
            "Note_table_ins_note() didn't set the retuned note ratio correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (ins_note_break_table)
{
    Real note_ratio;
    Real_init(&note_ratio);
    Note_table_ins_note(NULL, 0, &note_ratio);
}
END_TEST

START_TEST (ins_note_break_index1)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, -1, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_index2)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, INT_MIN, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_index3)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, NOTE_TABLE_NOTES, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_index4)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, INT_MAX, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_ratio1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, 0, NULL);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_ratio2)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 0, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_ratio3)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, -1, INT64_MAX);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_break_ratio4)
{
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, INT64_MIN, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note(table, 0, &note_ratio);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (del_note)
{
    int i = 0;
    int old_count = 0;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    Note_table_del_note(table, 0);
    fail_unless(table->note_count == 0,
            "Note_table_del_note() modified the note count of an empty table.");
    Note_table_del_note(table, NOTE_TABLE_NOTES - 1);
    fail_unless(table->note_count == 0,
            "Note_table_del_note() modified the note count of an empty table.");

    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        Note_table_set_note(table, i, &note_ratio);
    }
    assert(table->note_count == NOTE_TABLE_NOTES);
    old_count = table->note_count;
    Note_table_del_note(table, NOTE_TABLE_NOTES - 1);
    fail_unless(table->note_count == old_count - 1,
            "Note_table_del_note() didn't decrease the note count correctly.");

    old_count = table->note_count;
    Note_table_del_note(table, 8);
    fail_unless(table->note_count == old_count - 1,
            "Note_table_del_note() didn't decrease the note count correctly.");
    for (i = 0; i < NOTE_TABLE_NOTES - 2; ++i)
    {
        if (i < 8)
        {
            Real_init_as_frac(&note_ratio, i + 1, i + 2);
        }
        else
        {
            Real_init_as_frac(&note_ratio, i + 2, i + 3);
        }
        fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                "Note_table_del_note() didn't shift subsequent notes backward correctly.");
        fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                "Note_table_del_note() didn't shift subsequent notes backward correctly.");
    }

    old_count = table->note_count;
    Note_table_del_note(table, 0);
    fail_unless(table->note_count == old_count - 1,
            "Note_table_del_note() didn't decrease the note count correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (del_note_break_table)
{
    Note_table_del_note(NULL, 0);
}
END_TEST

START_TEST (del_note_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note(table, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note(table, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note(table, NOTE_TABLE_NOTES);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note(table, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (move_note)
{
    int i = 0;
    int actual_index = -1;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        Note_table_set_note(table, i, &note_ratio);
        Real_init_as_frac(&(table->notes[i].ratio_retuned), i + 2, i + 1);
    }
    assert(table->note_count == NOTE_TABLE_NOTES);

    actual_index = Note_table_move_note(table, 0, NOTE_TABLE_NOTES - 1);
    fail_unless(actual_index == NOTE_TABLE_NOTES - 1,
            "Note_table_move_note() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        if (i == NOTE_TABLE_NOTES - 1)
        {
            Real_init_as_frac(&note_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
            Real_init_as_frac(&note_ratio, 2, 1);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&note_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
            Real_init_as_frac(&note_ratio, i + 3, i + 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
        }
    }

    Note_table_del_note(table, 0);
    actual_index = Note_table_move_note(table, NOTE_TABLE_NOTES - 2, 0);
    fail_unless(actual_index == 0,
            "Note_table_move_note() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTES - 1; ++i)
    {
        if (i == 0)
        {
            Real_init_as_frac(&note_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
            Real_init_as_frac(&note_ratio, 2, 1);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&note_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
            Real_init_as_frac(&note_ratio, i + 3, i + 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't modify the note at new_index correctly.");
        }
    }

    actual_index = Note_table_move_note(table, NOTE_TABLE_NOTES - 1, 0); /* src is empty */
    fail_unless(actual_index == NOTE_TABLE_NOTES - 1,
            "Note_table_move_note() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTES - 1; ++i)
    {
        if (i == 0)
        {
            Real_init_as_frac(&note_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() incorrectly modified the table.");
            Real_init_as_frac(&note_ratio, 2, 1);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() incorrectly modified the table.");
        }
        else
        {
            Real_init_as_frac(&note_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() incorrectly modified the table.");
            Real_init_as_frac(&note_ratio, i + 3, i + 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() incorrectly modified the table.");
        }
    }

    actual_index = Note_table_move_note(table, 0, NOTE_TABLE_NOTES - 1); /* dest is empty */
    fail_unless(actual_index == NOTE_TABLE_NOTES - 2,
            "Note_table_move_note() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTES - 1; ++i)
    {
        if (i == NOTE_TABLE_NOTES - 2)
        {
            Real_init_as_frac(&note_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't handle empty note at new_index correctly.");
            Real_init_as_frac(&note_ratio, 2, 1);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't handle empty note at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&note_ratio, i + 3, i + 4);
            fail_unless(Real_cmp(&(table->notes[i].ratio), &note_ratio) == 0,
                    "Note_table_move_note() didn't handle empty note at new_index correctly.");
            Real_init_as_frac(&note_ratio, i + 4, i + 3);
            fail_unless(Real_cmp(&(table->notes[i].ratio_retuned), &note_ratio) == 0,
                    "Note_table_move_note() didn't handle empty note at new_index correctly.");
        }
    }

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (move_note_break_table)
{
    Note_table_move_note(NULL, 0, 1);
}
END_TEST

START_TEST (move_note_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, -1, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, INT_MIN, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, NOTE_TABLE_NOTES, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, INT_MAX, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_new_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, 0, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_new_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, 0, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_new_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, 0, NOTE_TABLE_NOTES);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_break_new_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note(table, 0, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif


START_TEST (get_note_ratio)
{
    int i = 0;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init(&note_ratio);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    fail_unless(Note_table_get_note_ratio(table, 0) == NULL,
            "Note_table_get_note_ratio() didn't return NULL for non-existent note.");
    
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        Note_table_set_note(table, i, &note_ratio);
        Real_init_as_frac(&(table->notes[i].ratio_retuned), i + 2, i + 1);
    }
    assert(table->note_count == NOTE_TABLE_NOTES);

    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        fail_unless(Real_cmp(Note_table_get_note_ratio(table, i), &note_ratio) == 0,
                "Note_table_get_note_ratio() didn't return the correct ratio.");
    }

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (get_note_ratio_break_table)
{
    Note_table_get_note_ratio(NULL, 0);
}
END_TEST

START_TEST (get_note_ratio_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_ratio(table, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_ratio_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_ratio(table, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_ratio_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_ratio(table, NOTE_TABLE_NOTES);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_ratio_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_ratio(table, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (set_note_mod)
{
    int i = 0;
    int actual_index = -1;
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 16, 15);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_set_note_mod() didn't set the modifier ratio correctly.");
    Real_init_as_frac(&mod_ratio, 1, 1);
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) != 0,
            "Note_table_set_note_mod() incorrectly copied the reference of the modifier ratio.");

    for (i = 1; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i, i + 1);
        actual_index = Note_table_set_note_mod(table, NOTE_TABLE_NOTE_MODS - 1, &mod_ratio);
        fail_unless(actual_index == i,
                "Note_table_set_note_mod() didn't return the correct index.");
        if (i < NOTE_TABLE_NOTE_MODS - 1)
        {
            fail_if(Real_cmp(&(table->note_mods[NOTE_TABLE_NOTE_MODS - 1].ratio), &mod_ratio) == 0,
                    "Note_table_set_note_mod() incorrectly set the modifier ratio at the end of table.");
        }
    }
    for (i = 1; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i, i + 1);
        fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                "Note_table_set_note_mod() didn't set the modifier ratio correctly.");
    }
    Real_init_as_frac(&mod_ratio, 16, 15);
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    
    Real_init_as_frac(&mod_ratio, 1, INT64_MAX);
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_set_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_frac(&mod_ratio, INT64_MAX, 1);
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_set_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_double(&mod_ratio, DBL_MIN);
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_set_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_double(&mod_ratio, DBL_MAX);
    actual_index = Note_table_set_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_set_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_set_note_mod() didn't set the modifier ratio correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_note_mod_break_table)
{
    Real mod_ratio;
    Real_init(&mod_ratio);
    Note_table_set_note_mod(NULL, 0, &mod_ratio);
}
END_TEST

START_TEST (set_note_mod_break_index1)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, -1, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_index2)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, INT_MIN, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_index3)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, NOTE_TABLE_NOTE_MODS, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_index4)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, INT_MAX, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_ratio1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, 0, NULL);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_ratio2)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 0, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_ratio3)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, -1, INT64_MAX);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (set_note_mod_break_ratio4)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, INT64_MIN, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_set_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (ins_note_mod)
{
    int i = 0;
    int actual_index = -1;
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 16, 15);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }
    
    actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't set the modifier ratio correctly.");
    Real_init_as_frac(&mod_ratio, 1, 1);
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) != 0,
            "Note_table_ins_note_mod() incorrectly copied the reference of the modifier ratio.");

    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 1; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
        fail_unless(actual_index == 0,
                "Note_table_ins_note_mod() didn't return the correct index.");
/*      fprintf(stderr, "# should be at index %d\n", i + 1); */
        if (i < NOTE_TABLE_NOTE_MODS - 2)
        {
            Real_init_as_frac(&mod_ratio, -1, 1);
            fail_unless(Real_cmp(&(table->note_mods[NOTE_TABLE_NOTE_MODS - 1].ratio), &mod_ratio) == 0,
                    "Note_table_ins_note_mod() put garbage after the inserted modifiers.");
        }
        else
        {
            Real_init_as_frac(&mod_ratio, 16, 15);
            fail_unless(Real_cmp(&(table->note_mods[NOTE_TABLE_NOTE_MODS - 1].ratio), &mod_ratio) == 0,
                    "Note_table_ins_note_mod() didn't shift modifiers forward correctly.");
        }
    }
    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 1; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        fail_unless(Real_cmp(&(table->note_mods[(NOTE_TABLE_NOTE_MODS - 2) - i].ratio), &mod_ratio) == 0,
                "Note_table_ins_note_mod() didn't shift modifiers forward correctly.");
    }
    Real_init_as_frac(&mod_ratio, 25, 24);
    actual_index = Note_table_ins_note_mod(table, 1, &mod_ratio);
    fail_unless(actual_index == 1,
            "Note_table_ins_note_mod() didn't return the correct index.");
    Real_init_as_frac(&mod_ratio, 1, 2);
    fail_unless(Real_cmp(&(table->note_mods[NOTE_TABLE_NOTE_MODS - 1].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't replace the last modifier correctly.");
    
    Real_init_as_frac(&mod_ratio, 1, INT64_MAX);
    actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_frac(&mod_ratio, INT64_MAX, 1);
    actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_double(&mod_ratio, DBL_MIN);
    actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't set the modifier ratio correctly.");
    
    Real_init_as_double(&mod_ratio, DBL_MAX);
    actual_index = Note_table_ins_note_mod(table, 0, &mod_ratio);
    fail_unless(actual_index == 0,
            "Note_table_ins_note_mod() didn't return the correct index.");
    fail_unless(Real_cmp(&(table->note_mods[0].ratio), &mod_ratio) == 0,
            "Note_table_ins_note_mod() didn't set the modifier ratio correctly.");

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (ins_note_mod_break_table)
{
    Real mod_ratio;
    Real_init(&mod_ratio);
    Note_table_ins_note_mod(NULL, 0, &mod_ratio);
}
END_TEST

START_TEST (ins_note_mod_break_index1)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, -1, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_index2)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, INT_MIN, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_index3)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, NOTE_TABLE_NOTE_MODS, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_index4)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 1, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, INT_MAX, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_ratio1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, 0, NULL);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_ratio2)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 0, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_ratio3)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, -1, INT64_MAX);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST

START_TEST (ins_note_mod_break_ratio4)
{
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, INT64_MIN, 1);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_ins_note_mod(table, 0, &mod_ratio);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (del_note_mod)
{
    int i = 0;
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 16, 15);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    Note_table_del_note_mod(table, 0);

    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        Note_table_set_note_mod(table, i, &mod_ratio);
    }
    Note_table_del_note_mod(table, NOTE_TABLE_NOTE_MODS - 1);

    Note_table_del_note_mod(table, 3);
    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 2; ++i)
    {
        if (i < 3)
        {
            Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        }
        else
        {
            Real_init_as_frac(&mod_ratio, i + 2, i + 3);
        }
        fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                "Note_table_del_note_mod() didn't shift subsequent modifiers backward correctly.");
    }

    Note_table_del_note_mod(table, 0);

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (del_note_mod_break_table)
{
    Note_table_del_note_mod(NULL, 0);
}
END_TEST

START_TEST (del_note_mod_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note_mod(table, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_mod_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note_mod(table, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_mod_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note_mod(table, NOTE_TABLE_NOTE_MODS);

    del_Note_table(table);
}
END_TEST

START_TEST (del_note_mod_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_del_note_mod(table, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (move_note_mod)
{
    int i = 0;
    int actual_index = -1;
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&mod_ratio, 16, 15);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        Note_table_set_note_mod(table, i, &mod_ratio);
    }

    actual_index = Note_table_move_note_mod(table, 0, NOTE_TABLE_NOTE_MODS - 1);
    fail_unless(actual_index == NOTE_TABLE_NOTE_MODS - 1,
            "Note_table_move_note_mod() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        if (i == NOTE_TABLE_NOTE_MODS - 1)
        {
            Real_init_as_frac(&mod_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't modify the modifier at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&mod_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't modify the modifier at new_index correctly.");
        }
    }

    Note_table_del_note_mod(table, 0);
    actual_index = Note_table_move_note_mod(table, NOTE_TABLE_NOTE_MODS - 2, 0);
    fail_unless(actual_index == 0,
            "Note_table_move_note_mod() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 1; ++i)
    {
        if (i == 0)
        {
            Real_init_as_frac(&mod_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't modify the modifier at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&mod_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't modify the modifier at new_index correctly.");
        }
    }

    actual_index = Note_table_move_note_mod(table, NOTE_TABLE_NOTE_MODS - 1, 0); /* src is empty */
    fail_unless(actual_index == NOTE_TABLE_NOTE_MODS - 1,
            "Note_table_move_note_mod() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 1; ++i)
    {
        if (i == 0)
        {
            Real_init_as_frac(&mod_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() incorrectly modified the table.");
        }
        else
        {
            Real_init_as_frac(&mod_ratio, i + 2, i + 3);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() incorrectly modified the table.");
        }
    }

    actual_index = Note_table_move_note_mod(table, 0, NOTE_TABLE_NOTE_MODS - 1); /* dest is empty */
    fail_unless(actual_index == NOTE_TABLE_NOTE_MODS - 2,
            "Note_table_move_note_mod() didn't return the correct index.");
    for (i = 0; i < NOTE_TABLE_NOTE_MODS - 1; ++i)
    {
        if (i == NOTE_TABLE_NOTE_MODS - 2)
        {
            Real_init_as_frac(&mod_ratio, 1, 2);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't handle empty modifier at new_index correctly.");
        }
        else
        {
            Real_init_as_frac(&mod_ratio, i + 3, i + 4);
            fail_unless(Real_cmp(&(table->note_mods[i].ratio), &mod_ratio) == 0,
                    "Note_table_move_note_mod() didn't handle empty modifier at new_index correctly.");
        }
    }

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (move_note_mod_break_table)
{
    Note_table_move_note_mod(NULL, 0, 1);
}
END_TEST

START_TEST (move_note_mod_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, -1, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, INT_MIN, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, NOTE_TABLE_NOTE_MODS, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, INT_MAX, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_new_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, 0, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_new_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, 0, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_new_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, 0, NOTE_TABLE_NOTE_MODS);

    del_Note_table(table);
}
END_TEST

START_TEST (move_note_mod_break_new_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_move_note_mod(table, 0, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif


START_TEST (get_note_mod_ratio)
{
    int i = 0;
    Real mod_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init(&mod_ratio);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    fail_unless(Note_table_get_note_mod_ratio(table, 0) == NULL,
            "Note_table_get_note_mod_ratio() didn't return NULL for non-existent modifier.");

    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        Note_table_set_note_mod(table, i, &mod_ratio);
    }

    for (i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        Real_init_as_frac(&mod_ratio, i + 1, i + 2);
        fail_unless(Real_cmp(Note_table_get_note_mod_ratio(table, i), &mod_ratio) == 0,
                "Note_table_get_note_mod_ratio() didn't return the correct ratio.");
    }

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (get_note_mod_ratio_break_table)
{
    Note_table_get_note_mod_ratio(NULL, 0);
}
END_TEST

START_TEST (get_note_mod_ratio_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_mod_ratio(table, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_mod_ratio_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_mod_ratio(table, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_mod_ratio_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_mod_ratio(table, NOTE_TABLE_NOTE_MODS);

    del_Note_table(table);
}
END_TEST

START_TEST (get_note_mod_ratio_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_note_mod_ratio(table, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (get_pitch)
{
    int i = 0;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    fail_unless(Note_table_get_pitch(table, 0, -1, NOTE_TABLE_MIDDLE_OCTAVE) < 0,
            "Note_table_get_pitch() didn't return a negative value for non-existent note.");

    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        Note_table_set_note(table, i, &note_ratio);
        Real_init_as_frac(&(table->notes[i].ratio_retuned), i + 2, i + 1);
    }
    assert(table->note_count == NOTE_TABLE_NOTES);
    Real_init_as_frac(&note_ratio, 16, 15);
    Note_table_set_note_mod(table, 0, &note_ratio);
    Real_init_as_frac(&note_ratio, 15, 16);
    Note_table_set_note_mod(table, 1, &note_ratio);

    /* without modifier */
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        double base_factor = 440;
        Real oct_mul;
        Real wrong_ratio;
        int j = 0;
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 1, 2);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE; j >= NOTE_TABLE_OCTAVE_FIRST; --j)
        {
            fail_if(Note_table_get_pitch(table, i, -1, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, -1, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            fail_unless(Note_table_get_pitch(table, i, -1, j)
                    - Note_table_get_pitch(table, i, INT_MIN, j) < 0.00000001,
                    "Note_table_get_pitch() returned different results for mod = -1 and mod = INT_MIN.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 2, 1);
        Real_mul(&note_ratio, &note_ratio, &oct_mul);
        Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE + 1; j <= NOTE_TABLE_OCTAVE_LAST; ++j)
        {
            fail_if(Note_table_get_pitch(table, i, -1, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, -1, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            fail_unless(Note_table_get_pitch(table, i, -1, j)
                    - Note_table_get_pitch(table, i, INT_MIN, j) < 0.00000001,
                    "Note_table_get_pitch() returned different results for mod = -1 and mod = INT_MIN.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
    }

    /* with modifier # */
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        double base_factor = 440;
        Real oct_mul;
        Real wrong_ratio;
        Real mod_ratio;
        int j = 0;
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 1, 2);
        Real_init_as_frac(&mod_ratio, 16, 15);
        Real_mul(&note_ratio, &note_ratio, &mod_ratio);
        Real_mul(&wrong_ratio, &wrong_ratio, &mod_ratio);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE; j >= NOTE_TABLE_OCTAVE_FIRST; --j)
        {
            fail_if(Note_table_get_pitch(table, i, 0, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, 0, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 2, 1);
        Real_mul(&note_ratio, &note_ratio, &oct_mul);
        Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        Real_mul(&note_ratio, &note_ratio, &mod_ratio);
        Real_mul(&wrong_ratio, &wrong_ratio, &mod_ratio);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE + 1; j <= NOTE_TABLE_OCTAVE_LAST; ++j)
        {
            fail_if(Note_table_get_pitch(table, i, 0, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, 0, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
    }

    /* with modifier b */
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        double base_factor = 440;
        Real oct_mul;
        Real wrong_ratio;
        Real mod_ratio;
        int j = 0;
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 1, 2);
        Real_init_as_frac(&mod_ratio, 15, 16);
        Real_mul(&note_ratio, &note_ratio, &mod_ratio);
        Real_mul(&wrong_ratio, &wrong_ratio, &mod_ratio);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE; j >= NOTE_TABLE_OCTAVE_FIRST; --j)
        {
            fail_if(Note_table_get_pitch(table, i, 1, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, 1, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
        Real_init_as_frac(&note_ratio, i + 2, i + 1);
        Real_init_as_frac(&wrong_ratio, i + 1, i + 2);
        Real_init_as_frac(&oct_mul, 2, 1);
        Real_mul(&note_ratio, &note_ratio, &oct_mul);
        Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        Real_mul(&note_ratio, &note_ratio, &mod_ratio);
        Real_mul(&wrong_ratio, &wrong_ratio, &mod_ratio);
        for (j = NOTE_TABLE_MIDDLE_OCTAVE + 1; j <= NOTE_TABLE_OCTAVE_LAST; ++j)
        {
            fail_if(Note_table_get_pitch(table, i, 1, j)
                    - Real_mul_float(&wrong_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't use the retuned ratios.");
            fail_unless(Note_table_get_pitch(table, i, 1, j)
                    - Real_mul_float(&note_ratio, base_factor) < 0.00000001,
                    "Note_table_get_pitch() didn't calculate the pitch accurately enough.");
            Real_mul(&note_ratio, &note_ratio, &oct_mul);
            Real_mul(&wrong_ratio, &wrong_ratio, &oct_mul);
        }
    }
}
END_TEST

#ifndef NDEBUG
START_TEST (get_pitch_break_table)
{
    Note_table_get_pitch(NULL, 0, -1, NOTE_TABLE_MIDDLE_OCTAVE);
}
END_TEST

START_TEST (get_pitch_break_index1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, -1, -1, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_index2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, INT_MIN, -1, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_index3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, NOTE_TABLE_NOTES, -1, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_index4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, INT_MAX, -1, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_mod1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, NOTE_TABLE_NOTE_MODS, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_mod2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, INT_MAX, NOTE_TABLE_MIDDLE_OCTAVE);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_octave1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, -1, NOTE_TABLE_OCTAVE_FIRST - 1);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_octave2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, -1, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_octave3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, -1, NOTE_TABLE_OCTAVE_LAST + 1);

    del_Note_table(table);
}
END_TEST

START_TEST (get_pitch_break_octave4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_get_pitch(table, 0, -1, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (retune)
{
    int i = 0;
    int k = 0;
    int new_ref = 0;
    Real fix_to_new_ratio;
    Real phafix_to_old_ratio;
    Real calculated;
    Real note_ratio;
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&note_ratio, 6, 5);
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(440, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    int nums[] =       { 1, 16, 9, 6, 5, 4, 45, 3, 8, 5, 9, 15 };
    int dens[] =       { 1, 15, 8, 5, 4, 3, 32, 2, 5, 3, 5,  8 };

    for (i = 0; i < 12; ++i)
    {
        Real_init_as_frac(&note_ratio, nums[i], dens[i]);
        Note_table_set_note(table, i, &note_ratio);
    }

    Note_table_retune(table, 2, 2);

    Real_init_as_frac(&note_ratio, 9, 8);
    Real_copy(&calculated, &(table->notes[2].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retain fixed_point.");
    Real_init_as_frac(&note_ratio, 144, 120);
    Real_copy(&calculated, &(table->notes[3].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 81, 64);
    Real_copy(&calculated, &(table->notes[4].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 54, 40);
    Real_copy(&calculated, &(table->notes[5].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 45, 32);
    Real_copy(&calculated, &(table->notes[6].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 36, 24);
    Real_copy(&calculated, &(table->notes[7].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 405, 256);
    Real_copy(&calculated, &(table->notes[8].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 27, 16);
    Real_copy(&calculated, &(table->notes[9].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 72, 40);
    Real_copy(&calculated, &(table->notes[10].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 45, 24);
    Real_copy(&calculated, &(table->notes[11].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 81, 80);
    Real_copy(&calculated, &(table->notes[0].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");
    Real_init_as_frac(&note_ratio, 135, 128);
    Real_copy(&calculated, &(table->notes[1].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");

    for (i = 0; i < 12; ++i)
    {
        Real_init_as_frac(&note_ratio, nums[i], dens[i]);
        Real_copy(&calculated, Note_table_get_note_ratio(table, i));
        fail_unless(Real_cmp(&calculated, &note_ratio) == 0,
                "Note_table_retune() incorrectly modified the original ratio"
                " from %d/%d to %lld/%lld.",
                nums[i], dens[i],
                Real_get_numerator(&calculated),
                Real_get_denominator(&calculated));
    }

    Note_table_retune(table, -1, 0);

    for (i = 0; i < 12; ++i)
    {
        Real_init_as_frac(&note_ratio, nums[i], dens[i]);
        fail_unless(Real_cmp(Note_table_get_note_ratio(table, i), &note_ratio) == 0,
                "Note_table_retune() incorrectly modified the original ratio"
                " from %d/%d to %lld/%lld.",
                nums[i], dens[i],
                Real_get_numerator(Note_table_get_note_ratio(table, i)),
                Real_get_denominator(Note_table_get_note_ratio(table, i)));
        fail_unless(Real_cmp(&note_ratio, &table->notes[i].ratio_retuned) == 0,
                "Note_table_retune() reset note incorrectly"
                " (expected %d/%d, got %lld/%lld).",
                nums[i], dens[i],
                Real_get_numerator(&table->notes[i].ratio_retuned),
                Real_get_denominator(&table->notes[i].ratio_retuned));
    }

    for (i = 0; i < 12; ++i)
    {
        fail_unless(Real_cmp(Note_table_get_note_ratio(table, i), &(table->notes[i].ratio_retuned)) == 0,
                "Note_table_retune() didn't reset correctly.");
    }

    // shift by Pythagorean comma
    for (i = 1; i <= 12; ++i)
    {
        int new_base = (7 * i) % 12;
        Note_table_retune(table, new_base, new_base);
    }

    for (i = 0; i < 12; ++i)
    {
        Real_init_as_frac(&note_ratio, 531441 * nums[i], 524288 * dens[i]);
        Real_copy(&calculated, &table->notes[i].ratio_retuned);
        fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
                "Note_table_retune() retuned note to %lld/%lld"
                " instead of %lld/%lld.",
                Real_get_numerator(&calculated),
                Real_get_denominator(&calculated),
                Real_get_numerator(&note_ratio),
                Real_get_denominator(&note_ratio));
    }

    Note_table_retune(table, -1, 0);

    Note_table_retune(table, 0, 0);

    for (i = 0; i < 12; ++i)
    {
        fail_unless(Real_cmp(Note_table_get_note_ratio(table, i), &(table->notes[i].ratio_retuned)) == 0,
                "Note_table_retune() didn't handle trivial retuning correctly.");
    }

    Note_table_retune(table, 16, 0);

    for (i = 0; i < 12; ++i)
    {
        fail_unless(Real_cmp(Note_table_get_note_ratio(table, i), &(table->notes[i].ratio_retuned)) == 0,
                "Note_table_retune() didn't handle trivial retuning correctly.");
    }
    
    for (i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        Real_init_as_frac(&note_ratio, i + 1, i + 2);
        Note_table_set_note(table, i, &note_ratio);
    }

    for (k = 1; k < NOTE_TABLE_NOTES; ++k)
    {
        int fixed_point = NOTE_TABLE_NOTES / 2; /* NOTE_TABLE_NOTES / 2; */
        new_ref = k;
        Note_table_retune(table, new_ref, fixed_point);
        Real_div(&fix_to_new_ratio, &(table->notes[fixed_point].ratio_retuned), &(table->notes[k].ratio_retuned));
        if (fixed_point < k && ((table->note_count + fixed_point - k) % table->note_count) > 0)
        {
            Real_mul(&fix_to_new_ratio, &fix_to_new_ratio, &octave_ratio);
        }
        Real_div(&phafix_to_old_ratio,
                Note_table_get_note_ratio(table, (table->note_count + fixed_point - k) % table->note_count),
                Note_table_get_note_ratio(table, 0));
        fail_unless(Real_cmp(&fix_to_new_ratio, &phafix_to_old_ratio) == 0,
                "Note_table_retune() didn't calculate new_ref correctly.");

        for (i = 1, new_ref = (new_ref + 1) % NOTE_TABLE_NOTES;
                i < NOTE_TABLE_NOTES;
                ++i, new_ref = (new_ref + 1) % NOTE_TABLE_NOTES)
        {
            Real orig;
            Real retuned;
            int prev = new_ref - 1;
            if (prev < 0)
            {
                prev = NOTE_TABLE_NOTES - 1;
            }
            Real_div(&orig, Note_table_get_note_ratio(table, i), Note_table_get_note_ratio(table, i - 1));
            Real_div(&retuned, &(table->notes[new_ref].ratio_retuned), &(table->notes[prev].ratio_retuned));
            if (new_ref == 0)
            {
                Real drop;
                Real_copy(&drop, &octave_ratio);
                Real_mul(&retuned, &retuned, &drop);
            }
/*          if (i == 73)
            {
                fprintf(stderr, "(n%d/n%d), (n%d/n%d): %ld/%ld, %ld/%ld; ", i, i - 1, new_ref, prev,
                        Real_get_numerator(&orig),
                        Real_get_denominator(&orig),
                        Real_get_numerator(&retuned),
                        Real_get_denominator(&retuned));
                fprintf(stderr, "n%d = %ld/%ld, n%d = %ld/%ld\n", new_ref,
                        Real_get_numerator(&(table->notes[new_ref].ratio_retuned)),
                        Real_get_denominator(&(table->notes[new_ref].ratio_retuned)),
                        prev,
                        Real_get_numerator(&(table->notes[prev].ratio_retuned)),
                        Real_get_denominator(&(table->notes[prev].ratio_retuned)));
            } */
/*          fprintf(stderr, "%d, %d\n", k, i); */
            if (Real_is_frac(&orig) && Real_is_frac(&retuned))
            {
                fail_unless(Real_cmp(&orig, &retuned) == 0,
                        "Note_table_retune() didn't retune correctly.");
            }
            else
            {
                fail_unless(Real_get_double(&orig) - Real_get_double(&retuned) < 0.00000001,
                        "Note_table_retune() didn't retune correctly.");
            }
        }
        Note_table_retune(table, -1, 0);
    }

    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (retune_break_table)
{
    Note_table_retune(NULL, 0, 0);
}
END_TEST

START_TEST (retune_break_new_ref1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, NOTE_TABLE_NOTES, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (retune_break_new_ref2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, INT_MAX, 0);

    del_Note_table(table);
}
END_TEST

START_TEST (retune_break_fixed_point1)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, 0, -1);

    del_Note_table(table);
}
END_TEST

START_TEST (retune_break_fixed_point2)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, 0, INT_MIN);

    del_Note_table(table);
}
END_TEST

START_TEST (retune_break_fixed_point3)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, 0, NOTE_TABLE_NOTES);

    del_Note_table(table);
}
END_TEST

START_TEST (retune_break_fixed_point4)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_retune(table, 0, INT_MAX);

    del_Note_table(table);
}
END_TEST
#endif

START_TEST (drift)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        abort();
    }

    Real note_ratio;
    Real_init_as_frac(&note_ratio, 1, 1);
    Note_table_set_note(table, 0, &note_ratio);
    Real_init_as_frac(&note_ratio, 16, 15);
    Note_table_set_note(table, 1, &note_ratio);
    Real_init_as_frac(&note_ratio, 9, 8);
    Note_table_set_note(table, 2, &note_ratio);
    Real_init_as_frac(&note_ratio, 6, 5);
    Note_table_set_note(table, 3, &note_ratio);
    Real_init_as_frac(&note_ratio, 5, 4);
    Note_table_set_note(table, 4, &note_ratio);
    Real_init_as_frac(&note_ratio, 4, 3);
    Note_table_set_note(table, 5, &note_ratio);
    Real_init_as_frac(&note_ratio, 45, 32);
    Note_table_set_note(table, 6, &note_ratio);
    Real_init_as_frac(&note_ratio, 3, 2);
    Note_table_set_note(table, 7, &note_ratio);
    Real_init_as_frac(&note_ratio, 8, 5);
    Note_table_set_note(table, 8, &note_ratio);
    Real_init_as_frac(&note_ratio, 5, 3);
    Note_table_set_note(table, 9, &note_ratio);
    Real_init_as_frac(&note_ratio, 9, 5);
    Note_table_set_note(table, 10, &note_ratio);
    Real_init_as_frac(&note_ratio, 15, 8);
    Note_table_set_note(table, 11, &note_ratio);

    Real calculated;
    Note_table_retune(table, 2, 2); // II -- 
    Real_init_as_frac(&note_ratio, 27, 20);
    Real_copy(&calculated, &(table->notes[5].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");

    Note_table_retune(table, 5, 5); // IV -- 
    Real_init_as_frac(&note_ratio, 243, 160);
    Real_copy(&calculated, &(table->notes[7].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");

    Note_table_retune(table, 7, 0); // V -- C retained for sustain
    Real_init_as_frac(&note_ratio, 81, 80);
    Real_copy(&calculated, &(table->notes[0].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");

    Note_table_retune(table, 0, 7); // I -- C: 81/80 -> drift by a syntonic comma
    Real_init_as_frac(&note_ratio, 81, 80);
    Real_copy(&calculated, &(table->notes[0].ratio_retuned));
    fail_unless(Real_cmp(&note_ratio, &calculated) == 0,
            "Note_table_retune() didn't retune correctly.");

    Note_table_drift(table, &note_ratio);

    int64_t num = Real_get_numerator(&note_ratio);
    int64_t den = Real_get_denominator(&note_ratio);
    fail_unless(num == 81 && den == 80,
            "Note_table_drift() returned %lld/%lld instead of 81/80.",
            (long long)num, (long long)den);
    
    // TODO: more tests
    
    del_Note_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (drift_break_table)
{
    Real drift;
    Note_table_drift(NULL, &drift);
}
END_TEST

START_TEST (drift_break_drift)
{
    Real octave_ratio;
    Note_table* table = NULL;
    Real_init_as_frac(&octave_ratio, 2, 1);
    table = new_Note_table(528, &octave_ratio);
    if (table == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- Out of memory?\n");
        return;
    }

    Note_table_drift(table, NULL);

    del_Note_table(table);
}
END_TEST
#endif


Suite* Note_table_suite(void)
{
    Suite* s = suite_create("Note_table");
    TCase* tc_new = tcase_create("new");
    TCase* tc_set_ref_pitch = tcase_create("set_ref_pitch");
    TCase* tc_get_ref_pitch = tcase_create("get_ref_pitch");
    TCase* tc_set_octave_ratio = tcase_create("set_octave_ratio");
    TCase* tc_get_octave_ratio = tcase_create("get_octave_ratio");
    TCase* tc_set_note = tcase_create("set_note");
    TCase* tc_ins_note = tcase_create("ins_note");
    TCase* tc_del_note = tcase_create("del_note");
    TCase* tc_move_note = tcase_create("move_note");
    TCase* tc_get_note_ratio = tcase_create("get_note_ratio");
    TCase* tc_set_note_mod = tcase_create("set_note_mod");
    TCase* tc_ins_note_mod = tcase_create("ins_note_mod");
    TCase* tc_del_note_mod = tcase_create("del_note_mod");
    TCase* tc_move_note_mod = tcase_create("move_note_mod");
    TCase* tc_get_note_mod_ratio = tcase_create("get_note_mod_ratio");
    TCase* tc_get_pitch = tcase_create("get_pitch");
    TCase* tc_retune = tcase_create("retune");
    TCase* tc_drift = tcase_create("drift");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_set_ref_pitch);
    suite_add_tcase(s, tc_get_ref_pitch);
    suite_add_tcase(s, tc_set_octave_ratio);
    suite_add_tcase(s, tc_get_octave_ratio);
    suite_add_tcase(s, tc_set_note);
    suite_add_tcase(s, tc_ins_note);
    suite_add_tcase(s, tc_del_note);
    suite_add_tcase(s, tc_move_note);
    suite_add_tcase(s, tc_get_note_ratio);
    suite_add_tcase(s, tc_set_note_mod);
    suite_add_tcase(s, tc_ins_note_mod);
    suite_add_tcase(s, tc_del_note_mod);
    suite_add_tcase(s, tc_move_note_mod);
    suite_add_tcase(s, tc_get_note_mod_ratio);
    suite_add_tcase(s, tc_get_pitch);
    suite_add_tcase(s, tc_retune);
    suite_add_tcase(s, tc_drift);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_set_ref_pitch, timeout);
    tcase_set_timeout(tc_get_ref_pitch, timeout);
    tcase_set_timeout(tc_set_octave_ratio, timeout);
    tcase_set_timeout(tc_get_octave_ratio, timeout);
    tcase_set_timeout(tc_set_note, timeout);
    tcase_set_timeout(tc_ins_note, timeout);
    tcase_set_timeout(tc_del_note, timeout);
    tcase_set_timeout(tc_move_note, timeout);
    tcase_set_timeout(tc_get_note_ratio, timeout);
    tcase_set_timeout(tc_set_note_mod, timeout);
    tcase_set_timeout(tc_ins_note_mod, timeout);
    tcase_set_timeout(tc_del_note_mod, timeout);
    tcase_set_timeout(tc_move_note_mod, timeout);
    tcase_set_timeout(tc_get_note_mod_ratio, timeout);
    tcase_set_timeout(tc_get_pitch, timeout);
    tcase_set_timeout(tc_retune, timeout);
    tcase_set_timeout(tc_drift, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_set_ref_pitch, set_ref_pitch);
    tcase_add_test(tc_get_ref_pitch, get_ref_pitch);
    tcase_add_test(tc_set_octave_ratio, set_octave_ratio);
    tcase_add_test(tc_get_octave_ratio, get_octave_ratio);
    tcase_add_test(tc_set_note, set_note);
    tcase_add_test(tc_ins_note, ins_note);
    tcase_add_test(tc_del_note, del_note);
    tcase_add_test(tc_move_note, move_note);
    tcase_add_test(tc_get_note_ratio, get_note_ratio);
    tcase_add_test(tc_set_note_mod, set_note_mod);
    tcase_add_test(tc_ins_note_mod, ins_note_mod);
    tcase_add_test(tc_del_note_mod, del_note_mod);
    tcase_add_test(tc_move_note_mod, move_note_mod);
    tcase_add_test(tc_get_note_mod_ratio, get_note_mod_ratio);
    tcase_add_test(tc_get_pitch, get_pitch);
    tcase_add_test(tc_retune, retune);
    tcase_add_test(tc_drift, drift);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break3, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break4, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break5, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break6, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break7, SIGABRT);

    tcase_add_test_raise_signal(tc_set_ref_pitch, set_ref_pitch_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_ref_pitch, set_ref_pitch_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_ref_pitch, set_ref_pitch_break3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_ref_pitch, set_ref_pitch_break4, SIGABRT);

    tcase_add_test_raise_signal(tc_get_ref_pitch, get_ref_pitch_break, SIGABRT);

    tcase_add_test_raise_signal(tc_set_octave_ratio, set_octave_ratio_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_octave_ratio, set_octave_ratio_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_octave_ratio, set_octave_ratio_break3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_octave_ratio, set_octave_ratio_break4, SIGABRT);
    tcase_add_test_raise_signal(tc_set_octave_ratio, set_octave_ratio_break5, SIGABRT);

    tcase_add_test_raise_signal(tc_get_octave_ratio, get_octave_ratio_break, SIGABRT);

    tcase_add_test_raise_signal(tc_set_note, set_note_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_ratio1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_ratio2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_ratio3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note, set_note_break_ratio4, SIGABRT);

    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_ratio1, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_ratio2, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_ratio3, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note, ins_note_break_ratio4, SIGABRT);

    tcase_add_test_raise_signal(tc_del_note, del_note_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note, del_note_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note, del_note_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note, del_note_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note, del_note_break_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_move_note, move_note_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_new_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_new_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_new_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note, move_note_break_new_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_get_note_ratio, get_note_ratio_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_ratio, get_note_ratio_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_ratio, get_note_ratio_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_ratio, get_note_ratio_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_ratio, get_note_ratio_break_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_ratio1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_ratio2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_ratio3, SIGABRT);
    tcase_add_test_raise_signal(tc_set_note_mod, set_note_mod_break_ratio4, SIGABRT);

    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_ratio1, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_ratio2, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_ratio3, SIGABRT);
    tcase_add_test_raise_signal(tc_ins_note_mod, ins_note_mod_break_ratio4, SIGABRT);

    tcase_add_test_raise_signal(tc_del_note_mod, del_note_mod_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note_mod, del_note_mod_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note_mod, del_note_mod_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note_mod, del_note_mod_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_del_note_mod, del_note_mod_break_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_new_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_new_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_new_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_move_note_mod, move_note_mod_break_new_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_get_note_mod_ratio, get_note_mod_ratio_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_mod_ratio, get_note_mod_ratio_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_mod_ratio, get_note_mod_ratio_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_mod_ratio, get_note_mod_ratio_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_get_note_mod_ratio, get_note_mod_ratio_break_index4, SIGABRT);

    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_index1, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_index2, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_index3, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_index4, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_mod1, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_mod2, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_octave1, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_octave2, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_octave3, SIGABRT);
    tcase_add_test_raise_signal(tc_get_pitch, get_pitch_break_octave4, SIGABRT);

    tcase_add_test_raise_signal(tc_retune, retune_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_new_ref1, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_new_ref2, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_fixed_point1, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_fixed_point2, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_fixed_point3, SIGABRT);
    tcase_add_test_raise_signal(tc_retune, retune_break_fixed_point4, SIGABRT);

    tcase_add_test_raise_signal(tc_drift, drift_break_table, SIGABRT);
    tcase_add_test_raise_signal(tc_drift, drift_break_drift, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Note_table_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    if (fail_count > 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


