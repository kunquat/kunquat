

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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


typedef struct Gen_group
{
    Gen_type active_type;
    Generator common_params;
    Generator* types[GEN_TYPE_LAST];
} Gen_group;


struct Instrument
{
    double default_force;       ///< Default force.
    double force_variation;     ///< Force variation.

    Scale** scales;             ///< The Scales of the Song.
    Scale*** default_scale;     ///< The default Scale of the Song.
    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Generators need.

    Gen_group gens[KQT_GENERATORS_MAX]; ///< Generators.
};


Instrument* new_Instrument(kqt_frame** bufs,
                           kqt_frame** vbufs,
                           kqt_frame** vbufs2,
                           int buf_count,
                           uint32_t buf_len,
                           Scale** scales,
                           Scale*** default_scale,
                           uint8_t events)
{
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs2 != NULL);
    assert(vbufs2[0] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(scales != NULL);
    assert(default_scale != NULL);
    assert(*default_scale != NULL);
    assert(*default_scale >= &scales[0]);
    assert(*default_scale <= &scales[KQT_SCALES_MAX - 1]);
    assert(events > 0);
    Instrument* ins = xalloc(Instrument);
    if (ins == NULL)
    {
        return NULL;
    }
    if (Instrument_params_init(&ins->params,
                               bufs, vbufs, vbufs2,
                               buf_count, buf_len,
                               default_scale) == NULL)
    {
        xfree(ins);
        return NULL;
    }

    ins->default_force = INS_DEFAULT_FORCE;
    ins->force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scales = scales;
    ins->default_scale = default_scale;
    ins->scale_index = INS_DEFAULT_SCALE_INDEX;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ins->gens[i].active_type = GEN_TYPE_NONE;
        if (!Generator_init(&ins->gens[i].common_params))
        {
            for (int k = i - 1; k >= 0; --k)
            {
                Generator_uninit(&ins->gens[k].common_params);
            }
            xfree(ins);
            return NULL;
        }
        for (int k = 0; k < GEN_TYPE_LAST; ++k)
        {
            ins->gens[i].types[k] = NULL;
        }
    }
    return ins;
}


bool Instrument_parse_header(Instrument* ins, char* str, Read_state* state)
{
    assert(ins != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    double default_force = INS_DEFAULT_FORCE;
    double force_variation = INS_DEFAULT_FORCE_VAR;
    int64_t scale_index = INS_DEFAULT_SCALE_INDEX;
    if (str != NULL)
    {
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
                    str = read_double(str, &default_force, state);
                }
                else if (strcmp(key, "force_variation") == 0)
                {
                    str = read_double(str, &force_variation, state);
                }
                else if (strcmp(key, "scale") == 0)
                {
                    str = read_int(str, &scale_index, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (scale_index < -1 || scale_index >= KQT_SCALES_MAX)
                    {
                        Read_state_set_error(state,
                                 "Invalid scale index: %" PRId64, scale_index);
                        return false;
                    }
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
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    ins->default_force = default_force;
    ins->force_variation = force_variation;
    Instrument_set_scale(ins, scale_index);
    return true;
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
    if (name[strlen(MAGIC_ID)] != 'i')
    {
        Read_state_set_error(state, "Directory is not an instrument file");
        return false;
    }
    if (strcmp(name + strlen(MAGIC_ID) + 1, KQT_FORMAT_VERSION) != 0)
    {
        Read_state_set_error(state, "Unsupported instrument version");
        return false;
    }
    File_tree* ins_tree = File_tree_get_child(tree, "p_instrument.json");
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
        if (!Instrument_parse_header(ins, str, state))
        {
            return false;
        }
    }
    Instrument_params_read(&ins->params, tree, state);
    if (state->error)
    {
        return false;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        char dir_name[] = "generator_xx";
        snprintf(dir_name, 13, "generator_%02x", i);
        File_tree* gen_tree = File_tree_get_child(tree, dir_name);
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
    return true;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


Generator* Instrument_get_common_gen_params(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return &ins->gens[index].common_params;
}


#if 0
int Instrument_get_gen_count(Instrument* ins)
{
    assert(ins != NULL);
    return ins->gen_count;
}
#endif


void Instrument_set_gen(Instrument* ins,
                        int index,
                        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    Instrument_set_gen_of_type(ins, index, gen);
    ins->gens[index].active_type = Generator_get_type(gen);
    return;
}


Generator* Instrument_get_gen(Instrument* ins,
                              int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return ins->gens[index].types[ins->gens[index].active_type];
}


void Instrument_set_gen_of_type(Instrument* ins,
                                int index,
                                Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    Gen_type type = Generator_get_type(gen);
    if (ins->gens[index].types[type] != NULL)
    {
        del_Generator(ins->gens[index].types[type]);
        ins->gens[index].types[type] = NULL;
    }
    ins->gens[index].types[type] = gen;
    return;
}


Generator* Instrument_get_gen_of_type(Instrument* ins,
                                      int index,
                                      Gen_type type)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    return ins->gens[index].types[type];
}


void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    Gen_type active_type = ins->gens[index].active_type;
    if (ins->gens[index].types[active_type] == NULL)
    {
        return;
    }
    del_Generator(ins->gens[index].types[active_type]);
    ins->gens[index].types[active_type] = NULL;
    ins->gens[index].active_type = GEN_TYPE_NONE;
    return;
}


void Instrument_set_scale(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= -1);
    assert(index < KQT_SCALES_MAX);
    if (index == -1 || true)
    {
        ins->params.scale = ins->default_scale;
    }
    else
    {
//        ins->params.scale = &ins->scales[index];
    }
    return;
}


#if 0
bool Instrument_add_event(Instrument* ins, Event* event, uint32_t pos)
{
    assert(ins != NULL);
    assert(event != NULL);
    bool ok = true;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (ins->gens[i] != NULL && ins->gens[i]->enabled)
        {
            ok &= Generator_add_event(ins->gens[i], event, pos);
        }
    }
    return ok;
}
#endif


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
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Gen_type active_type = ins->gens[i].active_type;
        if (ins->gens[i].types[active_type] != NULL)
        {
            Generator_mix(ins->gens[i].types[active_type],
                          &states[i], nframes, offset, freq, 120);
        }
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator_uninit(&ins->gens[i].common_params);
        for (int k = 0; k < GEN_TYPE_LAST; ++k)
        {
            if (ins->gens[i].types[k] != NULL)
            {
                del_Generator(ins->gens[i].types[k]);
            }
        }
    }
    xfree(ins);
    return;
}


