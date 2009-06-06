

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
#include <string.h>

#include <Instrument_params.h>
#include <File_base.h>
#include <File_tree.h>


#define new_env_or_fail(env, nodes, xmin, xmax, xstep, ymin, ymax, ystep) \
    do\
    {\
        (env) = new_Envelope((nodes), (xmin), (xmax), (xstep),\
                (ymin), (ymax), (ystep));\
        if ((env) == NULL)\
        {\
            Instrument_params_uninit(ip);\
            return NULL;\
        }\
    } while (false)

Instrument_params* Instrument_params_init(Instrument_params* ip,
        frame_t** bufs,
        frame_t** vbufs,
        int buf_count,
        uint32_t buf_len,
        Note_table** notes)
{
    assert(ip != NULL);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(bufs[1] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs[1] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(notes != NULL);
    ip->bufs = ip->gbufs = bufs;
    ip->buf_count = buf_count;
    ip->buf_len = buf_len;
    ip->pbufs = NULL;
    ip->vbufs = vbufs;
    ip->force_volume_env = NULL;
    ip->force_filter_env = NULL;
    ip->force_pitch_env = NULL;
    ip->volume_env = NULL;
    ip->volume_off_env = NULL;
    ip->pitch_pan_env = NULL;
    ip->filter_env = NULL;
    ip->filter_off_env = NULL;
    ip->notes = notes;

    ip->pedal = 0;
    ip->volume = 1;

    new_env_or_fail(ip->force_volume_env, 8,  0, 1, 0,  0, 1, 0);
    ip->force_volume_env_enabled = false;
    Envelope_set_node(ip->force_volume_env, 0, 0);
    Envelope_set_node(ip->force_volume_env, 1, 1);
    Envelope_set_first_lock(ip->force_volume_env, true, true);
    Envelope_set_last_lock(ip->force_volume_env, true, false);

    new_env_or_fail(ip->force_filter_env, 8,  0, 1, 0,  0, 1, 0);
    ip->force_filter_env_enabled = false;
    Envelope_set_node(ip->force_filter_env, 0, 1);
    Envelope_set_node(ip->force_filter_env, 1, 1);
    Envelope_set_first_lock(ip->force_filter_env, true, false);
    Envelope_set_last_lock(ip->force_filter_env, true, false);

    new_env_or_fail(ip->force_pitch_env, 8,  0, 1, 0,  -1, 1, 0);
    ip->force_pitch_env_enabled = false;
    Envelope_set_node(ip->force_pitch_env, 0, 0);
    Envelope_set_node(ip->force_pitch_env, 1, 0);
    Envelope_set_first_lock(ip->force_pitch_env, true, false);
    Envelope_set_last_lock(ip->force_pitch_env, true, false);

    new_env_or_fail(ip->volume_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->volume_env_enabled = false;
    ip->volume_env_carry = false;
    ip->volume_env_scale = 1;
    ip->volume_env_center = 440;
    Envelope_set_node(ip->volume_env, 0, 1);
    Envelope_set_node(ip->volume_env, 1, 1);
    Envelope_set_first_lock(ip->volume_env, true, false);

    new_env_or_fail(ip->volume_off_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->volume_off_env_enabled = false;
    ip->volume_off_env_factor = 1;
    ip->volume_off_env_center = 440;
    Envelope_set_node(ip->volume_off_env, 0, 1);
    Envelope_set_node(ip->volume_off_env, 1, 0);
    Envelope_set_first_lock(ip->volume_off_env, true, false);
    Envelope_set_last_lock(ip->volume_off_env, false, true);

    new_env_or_fail(ip->pitch_pan_env, 8,  -1, 1, 0,  -1, 1, 0);
    ip->pitch_pan_env_enabled = false;
    Envelope_set_node(ip->pitch_pan_env, -1, 0);
    Envelope_set_node(ip->pitch_pan_env, 0, 0);
    Envelope_set_node(ip->pitch_pan_env, 1, 0);
    Envelope_set_first_lock(ip->pitch_pan_env, true, false);
    Envelope_set_last_lock(ip->pitch_pan_env, true, false);

    new_env_or_fail(ip->filter_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->filter_env_enabled = false;
    ip->filter_env_scale = 1;
    ip->filter_env_center = 440;
    Envelope_set_node(ip->filter_env, 0, 1);
    Envelope_set_node(ip->filter_env, 1, 1);
    Envelope_set_first_lock(ip->filter_env, true, false);

    new_env_or_fail(ip->filter_off_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->filter_off_env_enabled = false;
    ip->filter_off_env_scale = 1;
    ip->filter_off_env_center = 440;
    Envelope_set_node(ip->filter_off_env, 0, 1);
    Envelope_set_node(ip->filter_off_env, 1, 1);
    Envelope_set_first_lock(ip->filter_off_env, true, false);

    return ip;
}

#undef new_env_or_fail


bool read_volume_off_env(Instrument_params* ip, File_tree* tree, Read_state* state)
{
    assert(ip != NULL);
    assert(tree != NULL);
    assert(!File_tree_is_dir(tree));
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    char* str = File_tree_get_data(tree);
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
        char key[128] = { '\0' };
        str = read_string(str, key, 128, state);
        str = read_const_char(str, ':', state);
        if (state->error)
        {
            return false;
        }
        if (strcmp(key, "enabled") == 0)
        {
            str = read_bool(str, &ip->volume_off_env_enabled, state);
        }
        else if (strcmp(key, "scale_factor") == 0)
        {
            str = read_double(str, &ip->volume_off_env_factor, state);
        }
        else if (strcmp(key, "scale_center") == 0)
        {
            str = read_double(str, &ip->volume_off_env_center, state);
        }
        else if (strcmp(key, "nodes") == 0)
        {
            str = Envelope_read(ip->volume_off_env, str, state);
        }
        else
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                     "Unrecognised key in Note Off volume envelope: %s", key);
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
            state->error = false;
            state->message[0] = '\0';
        }
    }
    str = read_const_char(str, '}', state);
    if (state->error)
    {
        return false;
    }
    return true;
}


bool Instrument_params_read(Instrument_params* ip, File_tree* tree, Read_state* state)
{
    assert(ip != NULL);
    assert(tree != NULL);
    assert(File_tree_is_dir(tree));
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    struct
    {
        char* name;
        bool (*read)(Instrument_params*, File_tree*, Read_state*);
    } files[] =
    {
        { "volume_off_env.json", read_volume_off_env },
        { NULL, NULL }
    };
    for (int i = 0; files[i].name != NULL; ++i)
    {
        assert(files[i].read != NULL);
        File_tree* obj_tree = File_tree_get_child(tree, files[i].name);
        if (obj_tree != NULL)
        {
            if (File_tree_is_dir(obj_tree))
            {
                state->error = true;
                snprintf(state->message, ERROR_MESSAGE_LENGTH,
                         "File %s is a directory", files[i].name);
                return false;
            }
            files[i].read(ip, obj_tree, state);
            if (state->error)
            {
                return false;
            }
        }
    }
    return true;
}


#define del_env_check(env)\
    do\
    {\
        if ((env) != NULL)\
        {\
            del_Envelope((env));\
            (env) = NULL;\
        }\
    } while (false)

void Instrument_params_uninit(Instrument_params* ip)
{
    assert(ip != NULL);
    del_env_check(ip->force_volume_env);
    del_env_check(ip->force_filter_env);
    del_env_check(ip->force_pitch_env);
    del_env_check(ip->volume_env);
    del_env_check(ip->volume_off_env);
    del_env_check(ip->pitch_pan_env);
    del_env_check(ip->filter_env);
    del_env_check(ip->filter_off_env);
    return;
}

#undef del_env_check


