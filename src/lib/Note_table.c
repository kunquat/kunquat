

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
#include <wchar.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <Note_table.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


#define NOTE_EXISTS(table, index) ((table)->notes[(index)].name[0] != '\0')
#define NOTE_MOD_EXISTS(table, index) ((table)->note_mods[(index)].name[0] != '\0')

#define MIN(a, b) ((a) < (b) ? (a) : (b))


Note_table* new_Note_table(wchar_t* name, pitch_t ref_pitch, Real* octave_ratio)
{
    assert(ref_pitch > 0);
    assert(octave_ratio != NULL);
    assert( Real_cmp(octave_ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    Note_table* table = xalloc(Note_table);
    if (table == NULL)
    {
        return NULL;
    }
    Note_table_clear(table);
    Note_table_set_name(table, name);
    table->ref_pitch = ref_pitch;
    Note_table_set_octave_ratio(table, octave_ratio);
    table->oct_ratio_cents = NAN;
    return table;
}


#define read_and_validate_tuning(str, ratio, cents, state)                        \
    do                                                                            \
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
                (state)->error = true;                                            \
                snprintf((state)->message, ERROR_MESSAGE_LENGTH,                  \
                         "Ratio is not positive");                                \
                return false;                                                     \
            }                                                                     \
        }                                                                         \
    } while (false)

bool Note_table_read(Note_table* table, File_tree* tree, Read_state* state)
{
    assert(table != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (!File_tree_is_dir(tree))
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                 "Note table is not a directory");
        return false;
    }
    const char* magic_id = "kunquat_";
    char* name = File_tree_get_name(tree);
    if (strncmp(name, magic_id, strlen(magic_id)) != 0)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                 "Directory is not a Kunquat file");
        return false;
    }
    if (name[strlen(magic_id)] != 't'
            || name[strlen(magic_id) + 1] != '_')
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                 "Directory is not a tuning file");
        return false;
    }
    const char* version = "00";
    if (strcmp(name + strlen(magic_id) + 2, version) != 0)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                 "Unsupported tuning version");
        return false;
    }
    File_tree* table_tree = File_tree_get_child(tree, "table.json");
    if (table_tree != NULL)
    {
        if (File_tree_is_dir(table_tree))
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                     "Note table specification is a directory");
            return false;
        }
        char* str = File_tree_get_data(table_tree);
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (!state->error)
        {
            return true;
        }
        state->error = false;
        state->message[0] = '\0';
        
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
                if (num < 0 || num >= NOTE_TABLE_NOTES)
                {
                    state->error = true;
                    snprintf(state->message, ERROR_MESSAGE_LENGTH,
                             "Invalid reference note number: %" PRId64, num);
                    return false;
                }
                table->ref_note = num;
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
                    state->error = true;
                    snprintf(state->message, ERROR_MESSAGE_LENGTH,
                             "Invalid reference pitch: %f", num);
                    return false;
                }
                table->ref_pitch = num;
            }
            else if (strcmp(key, "octave_ratio") == 0)
            {
                Real* ratio = Real_init(REAL_AUTO);
                double cents = NAN;
                read_and_validate_tuning(str, ratio, cents, state);
                if (!isnan(cents))
                {
                    Note_table_set_octave_ratio_cents(table, cents);
                }
                else
                {
                    Note_table_set_octave_ratio(table, ratio);
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
                    state->error = false;
                    state->message[0] = '\0';
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
                                Note_table_set_note_cents(table, count, L"-", cents);
                            }
                            else
                            {
                                Note_table_set_note_mod_cents(table, count, L"-", cents);
                            }
                        }
                        else
                        {
                            if (notes)
                            {
                                Note_table_set_note(table, count, L"-", ratio);
                            }
                            else
                            {
                                Note_table_set_note_mod(table, count, L"-", ratio);
                            }
                        }
                        ++count;
                        if (notes && count >= NOTE_TABLE_NOTES)
                        {
                            break;
                        }
                        else if (!notes && count >= NOTE_TABLE_NOTE_MODS)
                        {
                            break;
                        }
                        str = read_const_char(str, ',', state);
                        if (state->error)
                        {
                            expect_val = false;
                            state->error = false;
                            state->message[0] = '\0';
                        }
                    }
                    str = read_const_char(str, ']', state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (notes)
                    {
                        for (int i = count; i < NOTE_TABLE_NOTES; ++i)
                        {
                            table->notes[i].name[0] = L'\0';
                            table->notes[i].cents = NAN;
                            Real_init(&table->notes[i].ratio);
                            Real_init(&table->notes[i].ratio_retuned);
                        }
                    }
                }
            }
            else
            {
                state->error = true;
                snprintf(state->message, ERROR_MESSAGE_LENGTH,
                         "Unrecognised key in Note table: %s\n", key);
                return false;
            }
            str = read_const_char(str, ',', state);
            if (state->error)
            {
                expect_key = false;
                state->error = false;
                state->message[0] = '\0';
            }
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            return false;
        }
        if (table->ref_note >= table->note_count)
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                     "Reference note doesn't exist: %d\n", table->ref_note);
            return false;
        }
    }
    return true;
}

#undef read_and_validate_tuning


void Note_table_clear(Note_table* table)
{
    assert(table != NULL);
    Note_table_set_name(table, NULL);
    for (int i = 0; i < NOTE_TABLE_NOTE_MODS; ++i)
    {
        table->note_mods[i].name[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1]
                = table->note_mods[i].name[0] = L'\0';
        Real_init(&(table->note_mods[i].ratio));
    }
    for (int i = 0; i < NOTE_TABLE_NOTES; ++i)
    {
        table->notes[i].name[NOTE_TABLE_NOTE_NAME_MAX - 1]
                = table->notes[i].name[0] = L'\0';
        Real_init(&(table->notes[i].ratio));
        Real_init(&(table->notes[i].ratio_retuned));
    }
    table->note_count = 0;
    table->ref_note = 0;
    table->ref_note_retuned = 0;
    return;
}


void Note_table_set_name(Note_table* table, wchar_t* name)
{
    assert(table != NULL);
    table->name[NOTE_TABLE_NAME_MAX - 1] = table->name[0] = L'\0';
    if (name == NULL)
    {
        table->name[0] = L'\0';
        return;
    }
    wcsncpy(table->name, name, NOTE_TABLE_NAME_MAX - 1);
    table->name[NOTE_TABLE_NAME_MAX - 1] = L'\0';
    return;
}


wchar_t* Note_table_get_name(Note_table* table)
{
    assert(table != NULL);
    return table->name;
}


int Note_table_get_note_count(Note_table* table)
{
    assert(table != NULL);
    return table->note_count;
}


int Note_table_get_note_mod_count(Note_table* table)
{
    assert(table != NULL);
    int count = 0;
    while (count < NOTE_TABLE_NOTE_MODS && NOTE_MOD_EXISTS(table, count))
    {
        ++count;
    }
    return count;
}


bool Note_table_set_ref_note(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (index >= table->note_count)
    {
        return false;
    }
    table->ref_note = index;
    return true;
}


int Note_table_get_ref_note(Note_table* table)
{
    assert(table != NULL);
    return table->ref_note;
}


int Note_table_get_cur_ref_note(Note_table* table)
{
    assert(table != NULL);
    return table->ref_note_retuned;
}


void Note_table_set_ref_pitch(Note_table* table, pitch_t ref_pitch)
{
    assert(table != NULL);
    assert(ref_pitch > 0);
    table->ref_pitch = ref_pitch;
}


pitch_t Note_table_get_ref_pitch(Note_table* table)
{
    assert(table != NULL);
    return table->ref_pitch;
}


void Note_table_set_octave_ratio(Note_table* table, Real* octave_ratio)
{
    assert(table != NULL);
    assert(octave_ratio != NULL);
    assert( Real_cmp(octave_ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    Real_copy(&(table->octave_ratio), octave_ratio);
    Real_init_as_frac(&(table->oct_factors[NOTE_TABLE_OCTAVES / 2]), 1, 1);
    for (int i = NOTE_TABLE_MIDDLE_OCTAVE_UNBIASED - 1; i >= 0; --i)
    {
        Real_div(&(table->oct_factors[i]),
                &(table->oct_factors[i + 1]),
                &(table->octave_ratio));
    }
    for (int i = NOTE_TABLE_MIDDLE_OCTAVE_UNBIASED + 1; i < NOTE_TABLE_OCTAVES; ++i)
    {
        Real_mul(&(table->oct_factors[i]),
                &(table->oct_factors[i - 1]),
                &(table->octave_ratio));
    }
    table->oct_ratio_cents = NAN;
    return;
}


Real* Note_table_get_octave_ratio(Note_table* table)
{
    assert(table != NULL);
    return &(table->octave_ratio);
}


void Note_table_set_octave_ratio_cents(Note_table* table, double cents)
{
    assert(table != NULL);
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    Note_table_set_octave_ratio(table, ratio);
    table->oct_ratio_cents = cents;
    return;
}


double Note_table_get_octave_ratio_cents(Note_table* table)
{
    assert(table != NULL);
    return table->oct_ratio_cents;
}


int Note_table_set_note(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    while (index > 0 && !NOTE_EXISTS(table, index - 1))
    {
        assert(!NOTE_EXISTS(table, index));
        --index;
    }
    if (!NOTE_EXISTS(table, index))
    {
        if (table->note_count < NOTE_TABLE_NOTES)
        {
            ++(table->note_count);
        }
    }
    wcsncpy(table->notes[index].name, name, NOTE_TABLE_NOTE_NAME_MAX);
    table->notes[index].name[NOTE_TABLE_NOTE_NAME_MAX - 1] = L'\0';
    table->notes[index].cents = NAN;
    Real_copy(&(table->notes[index].ratio), ratio);
    Real_copy(&(table->notes[index].ratio_retuned), ratio);
    return index;
}


int Note_table_set_note_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Note_table_set_note(table, index, name, ratio);
    assert(actual_index >= 0);
    assert(actual_index < NOTE_TABLE_NOTES);
    table->notes[actual_index].cents = cents;
    return actual_index;
}


int Note_table_ins_note(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    if (!NOTE_EXISTS(table, index))
    {
        return Note_table_set_note(table, index, name, ratio);
    }
    if (table->note_count < NOTE_TABLE_NOTES)
    {
        ++(table->note_count);
    }
    int i = MIN(table->note_count, NOTE_TABLE_NOTES - 1);
/*    for (i = index; (i < NOTE_TABLE_NOTES - 1) && NOTE_EXISTS(table, i); ++i)
        ; */
    for (; i > index; --i)
    {
        wcsncpy(table->notes[i].name,
                table->notes[i - 1].name,
                NOTE_TABLE_NOTE_NAME_MAX);
        table->notes[i].name[NOTE_TABLE_NOTE_NAME_MAX - 1] = L'\0';
        table->notes[i].cents = table->notes[i - 1].cents;
        Real_copy(&(table->notes[i].ratio), &(table->notes[i - 1].ratio));
        Real_copy(&(table->notes[i].ratio_retuned),
                &(table->notes[i - 1].ratio_retuned));
    }
    assert(NOTE_EXISTS(table, MIN(table->note_count, NOTE_TABLE_NOTES - 1))
            == (table->note_count == NOTE_TABLE_NOTES));
    wcsncpy(table->notes[index].name, name, NOTE_TABLE_NOTE_NAME_MAX);
    table->notes[index].name[NOTE_TABLE_NOTE_NAME_MAX - 1] = L'\0';
    table->notes[index].cents = NAN;
    Real_copy(&(table->notes[index].ratio), ratio);
    Real_copy(&(table->notes[index].ratio_retuned), ratio);
    return index;
}


int Note_table_ins_note_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Note_table_ins_note(table, index, name, ratio);
    assert(actual_index >= 0);
    assert(actual_index < NOTE_TABLE_NOTES);
    table->notes[actual_index].cents = cents;
    return actual_index;
}


void Note_table_del_note(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return;
    }
    if (table->note_count > 0)
    {
        --(table->note_count);
        if ((table->ref_note >= table->note_count)
                && (table->ref_note > 0))
        {
            --(table->ref_note);
        }
        if ((table->ref_note_retuned >= table->note_count)
                && (table->ref_note_retuned > 0))
        {
            --(table->ref_note_retuned);
        }
    }
    int i = 0;
    for (i = index; (i < NOTE_TABLE_NOTES - 1) && NOTE_EXISTS(table, i + 1); ++i)
    {
        wcsncpy(table->notes[i].name,
                table->notes[i + 1].name,
                NOTE_TABLE_NOTE_NAME_MAX);
        table->notes[i].name[NOTE_TABLE_NOTE_NAME_MAX - 1] = L'\0';
        table->notes[i].cents = table->notes[i + 1].cents;
        Real_copy(&(table->notes[i].ratio), &(table->notes[i + 1].ratio));
        Real_copy(&(table->notes[i].ratio_retuned),
                &(table->notes[i + 1].ratio_retuned));
    }
    table->notes[i].name[0] = L'\0';
    table->notes[i].cents = NAN;
    Real_init(&(table->notes[i].ratio));
    Real_init(&(table->notes[i].ratio_retuned));
    return;
}


int Note_table_move_note(Note_table* table, int index, int new_index)
{
    // TODO: optimise?
    int actual_index = -1;
    wchar_t tmpname[NOTE_TABLE_NOTE_NAME_MAX];
    double tmpcents;
    Real tmpratio;
    Real tmpratio_retuned;
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(new_index >= 0);
    assert(new_index < NOTE_TABLE_NOTES);
    if (index == new_index)
    {
        return index;
    }
    if (!NOTE_EXISTS(table, index))
    {
        return index;
    }
    wcsncpy(tmpname, table->notes[index].name, NOTE_TABLE_NOTE_NAME_MAX);
    tmpname[NOTE_TABLE_NOTE_NAME_MAX - 1] = L'\0';
    tmpcents = table->notes[index].cents;
    Real_copy(&(tmpratio), &(table->notes[index].ratio));
    Real_copy(&(tmpratio_retuned), &(table->notes[index].ratio_retuned));
    Note_table_del_note(table, index);
    actual_index = Note_table_ins_note(table, new_index, tmpname, &tmpratio);
    if (NOTE_EXISTS(table, new_index))
    {
        Real_copy(&(table->notes[new_index].ratio_retuned), &tmpratio_retuned);
    }
    else
    {
        assert(table->note_count > 0);
        Real_copy(&(table->notes[table->note_count - 1].ratio_retuned),
                &tmpratio_retuned);
    }
    assert(actual_index >= 0);
    table->notes[actual_index].cents = tmpcents;
    return actual_index;
}


wchar_t* Note_table_get_note_name(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return NULL;
    }
    return table->notes[index].name;
}


Real* Note_table_get_note_ratio(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return NULL;
    }
    return &(table->notes[index].ratio);
}


Real* Note_table_get_cur_note_ratio(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return NULL;
    }
    return &table->notes[index].ratio_retuned;
}


double Note_table_get_note_cents(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return NAN;
    }
    return table->notes[index].cents;
}


double Note_table_get_cur_note_cents(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    if (!NOTE_EXISTS(table, index))
    {
        return NAN;
    }
    double val = Real_get_double(&table->notes[index].ratio_retuned);
    return log2(val) * 1200;
}


int Note_table_set_note_mod(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    while (index > 0 && !NOTE_MOD_EXISTS(table, index - 1))
    {
        assert(!NOTE_MOD_EXISTS(table, index));
        --index;
    }
    wcsncpy(table->note_mods[index].name, name, NOTE_TABLE_NOTE_MOD_NAME_MAX);
    table->note_mods[index].name[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1] = L'\0';
    table->note_mods[index].cents = NAN;
    Real_copy(&(table->note_mods[index].ratio), ratio);
    return index;
}


int Note_table_set_note_mod_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Note_table_set_note_mod(table, index, name, ratio);
    assert(actual_index >= 0);
    assert(actual_index < NOTE_TABLE_NOTE_MODS);
    table->note_mods[actual_index].cents = cents;
    return actual_index;
}


int Note_table_ins_note_mod(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(ratio != NULL);
    assert( Real_cmp(ratio, Real_init_as_frac(REAL_AUTO, 0, 1)) > 0 );
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return Note_table_set_note_mod(table, index, name, ratio);
    }
    int i = 0;
    for (i = index; (i < NOTE_TABLE_NOTE_MODS - 1)
            && NOTE_MOD_EXISTS(table, i); ++i)
        ;
    for (; i > index; --i)
    {
        wcsncpy(table->note_mods[i].name,
                table->note_mods[i - 1].name,
                NOTE_TABLE_NOTE_MOD_NAME_MAX);
        table->note_mods[i].name[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1] = L'\0';
        table->note_mods[i].cents = table->note_mods[i - 1].cents;
        Real_copy(&(table->note_mods[i].ratio),
                &(table->note_mods[i - 1].ratio));
    }
    wcsncpy(table->note_mods[index].name, name, NOTE_TABLE_NOTE_MOD_NAME_MAX);
    table->note_mods[index].name[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1] = L'\0';
    table->note_mods[index].cents = NAN;
    Real_copy(&(table->note_mods[index].ratio), ratio);
    return index;
}


int Note_table_ins_note_mod_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    assert(name != NULL);
    assert(name[0] != L'\0');
    assert(isfinite(cents));
    Real* ratio = Real_init_as_double(REAL_AUTO, exp2(cents / 1200));
    int actual_index = Note_table_ins_note_mod(table, index, name, ratio);
    assert(actual_index >= 0);
    assert(actual_index < NOTE_TABLE_NOTE_MODS);
    table->note_mods[actual_index].cents = cents;
    return actual_index;
}


void Note_table_del_note_mod(Note_table* table, int index)
{
    int i = 0;
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return;
    }
    for (i = index; (i < NOTE_TABLE_NOTE_MODS - 1)
            && NOTE_MOD_EXISTS(table, i + 1); ++i)
    {
        wcsncpy(table->note_mods[i].name,
                table->note_mods[i + 1].name,
                NOTE_TABLE_NOTE_MOD_NAME_MAX);
        table->note_mods[i].name[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1] = L'\0';
        table->note_mods[i].cents = table->note_mods[i + 1].cents;
        Real_copy(&(table->note_mods[i].ratio),
                &(table->note_mods[i + 1].ratio));
    }
    table->note_mods[i].name[0] = L'\0';
    table->note_mods[i].cents = NAN;
    Real_init(&(table->note_mods[i].ratio));
    return;
}


int Note_table_move_note_mod(Note_table* table, int index, int new_index)
{
    //* TODO: optimise?
    wchar_t tmpname[NOTE_TABLE_NOTE_MOD_NAME_MAX];
    double tmpcents;
    Real tmpratio;
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    assert(new_index >= 0);
    assert(new_index < NOTE_TABLE_NOTE_MODS);
    if (index == new_index)
    {
        return index;
    }
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return index;
    }
    wcsncpy(tmpname, table->note_mods[index].name, NOTE_TABLE_NOTE_MOD_NAME_MAX);
    tmpname[NOTE_TABLE_NOTE_MOD_NAME_MAX - 1] = L'\0';
    tmpcents = table->note_mods[index].cents;
    Real_copy(&(tmpratio), &(table->note_mods[index].ratio));
    Note_table_del_note_mod(table, index);
    int ret = Note_table_ins_note_mod(table, new_index, tmpname, &tmpratio);
    assert(ret >= 0);
    table->note_mods[ret].cents = tmpcents;
    return ret;
}


wchar_t* Note_table_get_note_mod_name(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return NULL;
    }
    return table->note_mods[index].name;
}


Real* Note_table_get_note_mod_ratio(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return NULL;
    }
    return &(table->note_mods[index].ratio);
}


double Note_table_get_note_mod_cents(Note_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTE_MODS);
    if (!NOTE_MOD_EXISTS(table, index))
    {
        return NAN;
    }
    return table->note_mods[index].cents;
}


pitch_t Note_table_get_pitch(Note_table* table,
        int index,
        int mod,
        int octave)
{
    octave -= NOTE_TABLE_OCTAVE_BIAS;
    Real final_ratio;
    assert(table != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLE_NOTES);
    assert(mod < NOTE_TABLE_NOTE_MODS);
    assert(octave >= 0);
    assert(octave < NOTE_TABLE_OCTAVES);
    if (!NOTE_EXISTS(table, index))
    {
        return -1;
    }
    Real_copy(&final_ratio,
            &(table->notes[index].ratio_retuned));
    if (mod >= 0 && NOTE_MOD_EXISTS(table, mod))
    {
        Real_mul(&final_ratio,
                &(table->notes[index].ratio_retuned),
                &(table->note_mods[mod].ratio));
    }
    Real_mul(&final_ratio,
            &final_ratio,
            &(table->oct_factors[octave]));
    return (pitch_t)Real_mul_float(&final_ratio, (double)(table->ref_pitch));
}


void Note_table_retune(Note_table* table, int new_ref, int fixed_point)
{
    assert(table != NULL);
    assert(new_ref < NOTE_TABLE_NOTES);
    assert(fixed_point >= 0);
    assert(fixed_point < NOTE_TABLE_NOTES);
    if (new_ref < 0)
    {
        // reset to original
        table->ref_note_retuned = table->ref_note;
        for (int i = 0; (i < NOTE_TABLE_NOTES) && NOTE_EXISTS(table, i); ++i)
        {
            Real_copy(&(table->notes[i].ratio_retuned),
                    &(table->notes[i].ratio));
        }
        return;
    }
    if ((new_ref == table->ref_note_retuned)
            || !NOTE_EXISTS(table, new_ref))
    {
        return;
    }
    if (!NOTE_EXISTS(table, fixed_point))
    {
        fixed_point = table->ref_note_retuned; // TODO: any better way?
    }
    // retune new_ref
    int fixed_new_order = fixed_point - new_ref;
    if (fixed_new_order < 0)
    {
        fixed_new_order += table->note_count;
    }
    assert(fixed_new_order >= 0);
    int fixed_counterpart = (table->ref_note_retuned + fixed_new_order)
            % table->note_count;
    Real fixed_to_new_ref_ratio;
    Real_div(&fixed_to_new_ref_ratio,
            &(table->notes[fixed_counterpart].ratio_retuned),
            &(table->notes[table->ref_note_retuned].ratio_retuned));
    if ((fixed_counterpart > table->ref_note_retuned)
            && (fixed_point < new_ref))
    {
        Real_div(&fixed_to_new_ref_ratio,
                &fixed_to_new_ref_ratio,
                &(table->octave_ratio));
    }
    else if ((fixed_counterpart < table->ref_note_retuned)
            && (fixed_point > new_ref))
    {
        Real_mul(&fixed_to_new_ref_ratio,
                &fixed_to_new_ref_ratio,
                &(table->octave_ratio));
    }
    static Real new_notes[NOTE_TABLE_NOTES];
    Real_div(&(new_notes[new_ref]),
            &(table->notes[fixed_point].ratio_retuned),
            &fixed_to_new_ref_ratio);
    /* new_ref is now retuned -- retune other notes excluding fixed_point */
    for (int i = 1; i < table->note_count; ++i)
    {
        Real to_ref_ratio;
        int cur_from_old_ref = (table->ref_note_retuned + i) % table->note_count;
        int cur_from_new_ref = (new_ref + i) % table->note_count;
        if (cur_from_new_ref == fixed_point)
        {
            Real_copy(&(new_notes[fixed_point]),
                    &(table->notes[fixed_point].ratio_retuned));
            continue;
        }
        Real_div(&to_ref_ratio,
                &(table->notes[cur_from_old_ref].ratio_retuned),
                &(table->notes[table->ref_note_retuned].ratio_retuned));
        if ((cur_from_new_ref > new_ref)
                && (cur_from_old_ref < table->ref_note_retuned))
        {
            Real_mul(&to_ref_ratio,
                    &to_ref_ratio,
                    &(table->octave_ratio));
        }
        else if ((cur_from_new_ref < new_ref)
                && (cur_from_old_ref > table->ref_note_retuned))
        {
            Real_div(&to_ref_ratio,
                    &to_ref_ratio,
                    &(table->octave_ratio));
        }
        Real_mul(&(new_notes[cur_from_new_ref]),
                &to_ref_ratio,
                &(new_notes[new_ref]));
    }
    /* update */
    table->ref_note_retuned = new_ref;
    for (int i = 0; i < table->note_count; ++i)
    {
        Real_copy(&(table->notes[i].ratio_retuned),
                &(new_notes[i]));
    }
    return;
}


Real* Note_table_drift(Note_table* table, Real* drift)
{
    assert(table != NULL);
    assert(drift != NULL);
    return Real_div(drift, &table->notes[table->ref_note].ratio_retuned,
            &table->notes[table->ref_note].ratio);
}


void del_Note_table(Note_table* table)
{
    assert(table != NULL);
    xfree(table);
    return;
}


