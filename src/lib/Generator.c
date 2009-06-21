

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

#include <Generator.h>
#include <Generator_sine.h>
#include <Generator_sawtooth.h>
#include <Generator_triangle.h>
#include <Generator_square.h>
#include <Generator_square303.h>
#include <Generator_pcm.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


void Generator_init(Generator* gen)
{
    assert(gen != NULL);
    gen->enabled = true;
    gen->volume_dB = 0;
    gen->volume = 1;
    return;
}


Generator* new_Generator_from_file_tree(File_tree* tree,
                                        Read_state* state,
                                        Instrument_params* ins_params)
{
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Generator is not a directory");
        return NULL;
    }
    File_tree* type_tree = File_tree_get_child(tree, "gen_type.json");
    if (type_tree == NULL)
    {
        Read_state_set_error(state, "Generator does not contain a type description");
        return NULL;
    }
    Read_state_init(state, File_tree_get_path(type_tree));
    if (File_tree_is_dir(type_tree))
    {
        Read_state_set_error(state, "Type description of the Generator is a directory");
        return NULL;
    }
    char* str = File_tree_get_data(type_tree);
    char type_str[128] = { '\0' };
    str = read_string(str, type_str, 128, state);
    if (state->error)
    {
        return NULL;
    }
    Generator* gen = NULL;
    if (strcmp(type_str, "sine") == 0)
    {
        gen = (Generator*)new_Generator_sine(ins_params);
    }
    else if (strcmp(type_str, "sawtooth") == 0)
    {
        gen = (Generator*)new_Generator_sawtooth(ins_params);
    }
    else if (strcmp(type_str, "triangle") == 0)
    {
        gen = (Generator*)new_Generator_triangle(ins_params);
    }
    else if (strcmp(type_str, "square") == 0)
    {
        gen = (Generator*)new_Generator_square(ins_params);
    }
    else if (strcmp(type_str, "square303") == 0)
    {
        gen = (Generator*)new_Generator_square303(ins_params);
    }
    else if (strcmp(type_str, "pcm") == 0)
    {
        gen = (Generator*)new_Generator_pcm(ins_params);
    }
    else
    {
        Read_state_set_error(state, "Unsupported Generator type: %s", type_str);
        return NULL;
    }
    if (gen == NULL)
    {
        Read_state_set_error(state, "Couldn't allocate memory for the new Generator");
        return NULL;
    }

    File_tree* info_tree = File_tree_get_child(tree, "info_gen.json");
    if (info_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(info_tree));
        if (File_tree_is_dir(info_tree))
        {
            del_Generator(gen);
            Read_state_set_error(state,
                     "Field description of the Generator is a directory");
            return NULL;
        }
        str = File_tree_get_data(info_tree);
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            del_Generator(gen);
            return NULL;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            char key[128] = { '\0' };
            bool expect_key = true;
            while (expect_key)
            {
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    del_Generator(gen);
                    return NULL;
                }
                if (strcmp(key, "enabled") == 0)
                {
                    str = read_bool(str, &gen->enabled, state);
                }
                else if (strcmp(key, "volume") == 0)
                {
                    str = read_double(str, &gen->volume_dB, state);
                    if (!state->error)
                    {
                        gen->volume = exp2(gen->volume_dB / 6);
                    }
                }
                else
                {
                    del_Generator(gen);
                    Read_state_set_error(state,
                             "Unsupported key in Generator info: %s", key);
                    return NULL;
                }
                if (state->error)
                {
                    del_Generator(gen);
                    return NULL;
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
                del_Generator(gen);
                return NULL;
            }
        }
    }

    assert(gen->read != NULL);
    gen->read(gen, tree, state);
    if (state->error)
    {
        del_Generator(gen);
        return NULL;
    }
    return gen;
}


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_process_note(Generator* gen,
                            Voice_state* state,
                            int note,
                            int mod,
                            int octave)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(note >= 0);
    assert(note < NOTE_TABLE_NOTES);
    assert(mod < NOTE_TABLE_NOTE_MODS);
    assert(octave >= NOTE_TABLE_OCTAVE_FIRST);
    assert(octave <= NOTE_TABLE_OCTAVE_LAST);
    if (gen->ins_params->notes == NULL || *gen->ins_params->notes == NULL)
    {
        return;
    }
    pitch_t freq = Note_table_get_pitch(*gen->ins_params->notes, note, mod, octave);
    if (freq > 0)
    {
        state->freq = freq;
    }
    return;
}


void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    uint32_t mixed = offset;
    while (mixed < nframes)
    {
        mixed = gen->mix(gen, state, nframes, mixed, freq,
                         gen->ins_params->buf_count,
                         gen->ins_params->bufs);
        if (!state->active)
        {
            break;
        }
    }
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


