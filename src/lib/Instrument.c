

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
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Generator.h>
#include <Instrument.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


Instrument* new_Instrument(frame_t** bufs,
                           frame_t** vbufs,
                           int buf_count,
                           uint32_t buf_len,
                           Note_table** note_tables,
                           Note_table** default_notes,
                           uint8_t events)
{
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(bufs[1] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs[1] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(note_tables != NULL);
    assert(default_notes != NULL);
    assert(default_notes >= &note_tables[0]);
    assert(default_notes <= &note_tables[NOTE_TABLES_MAX - 1]);
    assert(events > 0);
    Instrument* ins = xalloc(Instrument);
    if (ins == NULL)
    {
        return NULL;
    }
    if (Instrument_params_init(&ins->params,
                               bufs, vbufs,
                               buf_count, buf_len,
                               default_notes) == NULL)
    {
        xfree(ins);
        return NULL;
    }
    ins->events = NULL;
    ins->events = new_Event_queue(events);
    if (ins->events == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }

    ins->default_force = 1;
    ins->force_variation = 0;

    ins->note_tables = note_tables;
    ins->default_notes = default_notes;
    ins->notes_index = -1;

    ins->gen_count = 0;
    for (int i = 0; i < GENERATORS_MAX; ++i)
    {
        ins->gens[i] = NULL;
    }

    ins->name[0] = ins->name[INS_NAME_MAX - 1] = L'\0';
    return ins;
}


bool Instrument_read(Instrument* ins, File_tree* tree, Read_state* state)
{
    assert(ins != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Instrument is not a directory");
        return false;
    }
    char* name = File_tree_get_name(tree);
    if (strncmp(name, MAGIC_ID, strlen(MAGIC_ID)) != 0)
    {
        Read_state_set_error(state, "Directory is not a Kunquat file");
        return false;
    }
    if (name[strlen(MAGIC_ID)] != 'i'
             || name[strlen(MAGIC_ID) + 1] != '_')
    {
        Read_state_set_error(state, "Directory is not an instrument file");
        return false;
    }
    const char* version = "00";
    if (strcmp(name + strlen(MAGIC_ID) + 2, version) != 0)
    {
        Read_state_set_error(state, "Unsupported instrument version");
        return false;
    }
    File_tree* ins_tree = File_tree_get_child(tree, "info_ins.json");
    if (ins_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(ins_tree));
        if (File_tree_is_dir(ins_tree))
        {
            Read_state_set_error(state,
                     "Instrument information file is a directory");
            return false;
        }
        char* str = File_tree_get_data(ins_tree);
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            bool expect_key = true;
            while (expect_key)
            {
                char key[128] = { '\0' };
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (strcmp(key, "force") == 0)
                {
                    str = read_double(str, &ins->default_force, state);
                }
                else if (strcmp(key, "force_variation") == 0)
                {
                    str = read_double(str, &ins->force_variation, state);
                }
                else if (strcmp(key, "note_table") == 0)
                {
                    int64_t num = -1;
                    str = read_int(str, &num, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (num < -1 || num >= NOTE_TABLES_MAX)
                    {
                        Read_state_set_error(state,
                                 "Invalid Note table index: %" PRId64, num);
                        return false;
                    }
                    Instrument_set_note_table(ins, num);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unsupported field in instrument information: %s", key);
                    return false;
                }
                if (state->error)
                {
                    return false;
                }
                str = read_const_char(str, ',', state);
                if (state->error)
                {
                    expect_key = false;
                    Read_state_clear_error(state);
                }
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    Instrument_params_read(&ins->params, tree, state);
    if (state->error)
    {
        return false;
    }
    File_tree* gens_tree = File_tree_get_child(tree, "gens");
    if (gens_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(gens_tree));
        if (!File_tree_is_dir(gens_tree))
        {
            Read_state_set_error(state,
                     "Generator collection is not a directory");
            return false;
        }
        for (int i = 0; i < GENERATORS_MAX; ++i)
        {
            char dir_name[] = "g_00";
            snprintf(dir_name, 5, "g_%02x", i);
            File_tree* gen_tree = File_tree_get_child(gens_tree, dir_name);
            if (gen_tree != NULL)
            {
                Generator* gen = new_Generator_from_file_tree(gen_tree, state,
                                 Instrument_get_params(ins));
                if (state->error)
                {
                    assert(gen == NULL);
                    return false;
                }
                assert(gen != NULL);
                Instrument_set_gen(ins, i, gen);
            }
        }
    }
    return true;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


int Instrument_get_gen_count(Instrument* ins)
{
    assert(ins != NULL);
    return ins->gen_count;
}


int Instrument_set_gen(Instrument* ins,
        int index,
        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    assert(gen != NULL);
    if (ins->gens[index] != NULL)
    {
        del_Generator(ins->gens[index]);
        ins->gens[index] = NULL;
    }
    else
    {
        ++ins->gen_count;
    }
    while (index > 0 && ins->gens[index - 1] == NULL)
    {
        --index;
    }
    ins->gens[index] = gen;
    return index;
}


Generator* Instrument_get_gen(Instrument* ins,
        int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    return ins->gens[index];
}


void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    if (ins->gens[index] == NULL)
    {
        return;
    }
    --ins->gen_count;
    del_Generator(ins->gens[index]);
    ins->gens[index] = NULL;
    while (index < GENERATORS_MAX - 1 && ins->gens[index + 1] != NULL)
    {
        ins->gens[index] = ins->gens[index + 1];
        ins->gens[index + 1] = NULL;
        ++index;
    }
    return;
}


void Instrument_set_name(Instrument* ins, wchar_t* name)
{
    assert(ins != NULL);
    assert(name != NULL);
    wcsncpy(ins->name, name, INS_NAME_MAX - 1);
    ins->name[INS_NAME_MAX - 1] = L'\0';
    return;
}


wchar_t* Instrument_get_name(Instrument* ins)
{
    assert(ins != NULL);
    return ins->name;
}


void Instrument_set_note_table(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= -1);
    assert(index < NOTE_TABLES_MAX);
    if (index == -1)
    {
        ins->params.notes = ins->default_notes;
    }
    else
    {
        ins->params.notes = &ins->note_tables[index];
    }
    return;
}


void Instrument_mix(Instrument* ins,
        Voice_state* states,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(ins != NULL);
    assert(states != NULL);
//  assert(nframes <= ins->buf_len);
    assert(freq > 0);
    for (int i = 0; i < GENERATORS_MAX && ins->gens[i] != NULL; ++i)
    {
        Generator_mix(ins->gens[i], &states[i], nframes, offset, freq);
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
    if (ins->events != NULL)
    {
        del_Event_queue(ins->events);
    }
    for (int i = 0; i < GENERATORS_MAX && ins->gens[i] != NULL; ++i)
    {
        del_Generator(ins->gens[i]);
    }
    xfree(ins);
}


