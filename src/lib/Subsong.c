

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
#include <math.h>

#include <Song_limits.h>
#include <File_base.h>
#include <File_tree.h>
#include <Subsong.h>

#include <xmemory.h>


Subsong* new_Subsong(void)
{
    Subsong* ss = xalloc(Subsong);
    if (ss == NULL)
    {
        return NULL;
    }
    ss->res = 8;
    ss->pats = xnalloc(int16_t, ss->res);
    if (ss->pats == NULL)
    {
        xfree(ss);
        return NULL;
    }
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = ORDER_NONE;
    }
    ss->tempo = 120;
    ss->global_vol = -4;
    ss->notes = 0;
    return ss;
}


bool Subsong_read(Subsong* ss, File_tree* tree, Read_state* state)
{
    assert(ss != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Subsong is not a directory");
        return false;
    }
    File_tree* info = File_tree_get_child(tree, "info_sub.json");
    if (info == NULL)
    {
        return true;
    }
    Read_state_init(state, File_tree_get_path(info));
    if (File_tree_is_dir(info))
    {
        Read_state_set_error(state, "Subsong description is a directory");
        return false;
    }
    char* str = File_tree_get_data(info);
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
    Read_state_clear_error(state);
    char key[128] = { '\0' };
    bool expect_pair = true;
    while (expect_pair)
    {
        str = read_string(str, key, 128, state);
        str = read_const_char(str, ':', state);
        if (state->error)
        {
            return false;
        }
        if (strcmp(key, "tempo") == 0)
        {
            str = read_double(str, &ss->tempo, state);
        }
        else if (strcmp(key, "global_vol") == 0)
        {
            str = read_double(str, &ss->global_vol, state);
        }
        else if (strcmp(key, "notes") == 0)
        {
            int64_t num = 0;
            str = read_int(str, &num, state);
            ss->notes = num;
        }
        else if (strcmp(key, "patterns") == 0)
        {
            str = read_const_char(str, '[', state);
            if (state->error)
            {
                return false;
            }
            str = read_const_char(str, ']', state);
            if (state->error)
            {
                Read_state_clear_error(state);
                bool expect_num = true;
                int index = 0;
                while (expect_num && index < ORDERS_MAX)
                {
                    int64_t num = 0;
                    str = read_int(str, &num, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if ((num < 0 || num >= PATTERNS_MAX) && num != ORDER_NONE)
                    {
                        Read_state_set_error(state,
                                 "Pattern number (%" PRId64 ") is outside valid range", num);
                        return false;
                    }
                    if (!Subsong_set(ss, index, num))
                    {
                        Read_state_set_error(state,
                                 "Couldn't allocate memory for a Subsong");
                        return false;
                    }
                    ++index;
                    str = read_const_char(str, ',', state);
                    if (state->error)
                    {
                        Read_state_clear_error(state);
                        expect_num = false;
                    }
                }
                str = read_const_char(str, ']', state);
                if (state->error)
                {
                    return false;
                }
            }
        }
        else
        {
            Read_state_set_error(state, "Unrecognised key in Subsong: %s\n", key);
            return false;
        }
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, ',', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            expect_pair = false;
        }
    }
    str = read_const_char(str, '}', state);
    if (state->error)
    {
        return false;
    }
    return true;
}


bool Subsong_set(Subsong* ss, int index, int16_t pat)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < ORDERS_MAX);
    assert(pat >= 0 || pat == ORDER_NONE);
    if (index >= ss->res)
    {
        int new_res = ss->res << 1;
        if (index >= new_res)
        {
            new_res = index + 1;
        }
        int16_t* new_pats = xrealloc(int16_t, new_res, ss->pats);
        if (new_pats == NULL)
        {
            return false;
        }
        ss->pats = new_pats;
        for (int i = ss->res; i < new_res; ++i)
        {
            ss->pats[i] = ORDER_NONE;
        }
        ss->res = new_res;
    }
    ss->pats[index] = pat;
    return true;
}


int16_t Subsong_get(Subsong* ss, int index)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < ORDERS_MAX);
    if (index >= ss->res)
    {
        return ORDER_NONE;
    }
    return ss->pats[index];
}


void Subsong_set_tempo(Subsong* ss, double tempo)
{
    assert(ss != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);
    ss->tempo = tempo;
    return;
}


double Subsong_get_tempo(Subsong* ss)
{
    assert(ss != NULL);
    return ss->tempo;
}


void Subsong_set_global_vol(Subsong* ss, double vol)
{
    assert(ss != NULL);
    assert(isfinite(vol) || vol == -INFINITY);
    ss->global_vol = vol;
    return;
}


double Subsong_get_global_vol(Subsong* ss)
{
    assert(ss != NULL);
    return ss->global_vol;
}


#if 0
void Subsong_clear(Subsong* ss)
{
    assert(ss != NULL);
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = ORDER_NONE;
    }
    return;
}
#endif


void del_Subsong(Subsong* ss)
{
    assert(ss != NULL);
    xfree(ss->pats);
    xfree(ss);
    return;
}


