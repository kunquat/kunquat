

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
#include <stdio.h>

#include "Listener.h"
#include "Listener_ins.h"
#include "Listener_ins_pcm.h"
#include "utf8.h"

#include <Song.h>
#include <Ins_table.h>
#include <Generator_debug.h>
#include <Generator_sine.h>
#include <Generator_pcm.h>
#include <Instrument.h>
#include <Song_limits.h>


int Listener_get_insts(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    Ins_table* table = Song_get_insts(song);
    for (int i = 1; i <= INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(table, i);
        if (ins != NULL)
        {
            if (!ins_info(lr, song_id, i, ins))
            {
                return 0;
            }
        }
    }
    return 0;
}


int Listener_new_ins(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(argv != NULL);
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t ins_num = argv[1]->i;
    check_cond(lr, ins_num >= 1 && ins_num <= INSTRUMENTS_MAX,
            "The Instrument number (%ld)", (long)ins_num);
    int32_t ins_type = argv[2]->i;
    check_cond(lr, ins_type >= GEN_TYPE_NONE && ins_type < GEN_TYPE_LAST,
            "The Instrument type (%ld)", (long)ins_type);
    if (argv[2]->i > GEN_TYPE_PCM)
    {
        strcpy(lr->method_path + lr->host_path_len, "error");
        lo_send(lr->host, lr->method_path, "s", "Only debug, sine and pcm instruments supported");
        return 0;
    }
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    Ins_table* table = Song_get_insts(song);
    assert(table != NULL);
    Instrument* ins = NULL;
    if (ins_type != GEN_TYPE_NONE)
    {
        ins = new_Instrument(Song_get_bufs(song),
                Song_get_buf_size(song),
                32); // XXX: get event count from the configuration
        if (ins == NULL)
        {
            send_memory_fail(lr, "the new Instrument");
        }
        Generator* gen = NULL;
        switch (ins_type)
        {
            case GEN_TYPE_DEBUG:
            {
                Generator_debug* gen_debug = new_Generator_debug(Instrument_get_params(ins));
                if (gen_debug == NULL)
                {
                    send_memory_fail(lr, "the Generator of the new Instrument");
                }
                gen = (Generator*)gen_debug;
            }
            break;
            case GEN_TYPE_SINE:
            {
                Generator_sine* gen_sine = new_Generator_sine(Instrument_get_params(ins));
                if (gen_sine == NULL)
                {
                    send_memory_fail(lr, "the Generator of the new Instrument");
                }
                gen = (Generator*)gen_sine;
            }
            break;
            case GEN_TYPE_PCM:
            {
                Generator_pcm* gen_pcm = new_Generator_pcm(Instrument_get_params(ins));
                if (gen_pcm == NULL)
                {
                    send_memory_fail(lr, "the Generator of the new Instrument");
                }
                gen = (Generator*)gen_pcm;
            }
            break;
        }
        assert(gen != NULL);
        Instrument_set_gen(ins, 0, gen);
        Instrument_set_note_table(ins, Song_get_active_notes(song));
        if (!Ins_table_set(table, ins_num, ins))
        {
            del_Instrument(ins);
            send_memory_fail(lr, "the new Instrument");
        }
    }
    else
    {
        Ins_table_remove(table, ins_num);
    }
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


int Listener_ins_set_name(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(argv != NULL);
    assert(&argv[2]->s != NULL);
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    Instrument* ins = NULL;
    if (!ins_get(lr, argv[0]->i, argv[1]->i, &ins))
    {
        return 0;
    }
    if (ins != NULL)
    {
        wchar_t name[INS_NAME_MAX] = { L'\0' };
        unsigned char* src = (unsigned char*)&argv[2]->s;
        from_utf8_check(lr, name, src, INS_NAME_MAX,
                "the name of the Instrument");
        Instrument_set_name(ins, name);
    }
    ins_info(lr, lr->player_cur->id, argv[1]->i, ins);
    return 0;
}


int Listener_del_ins(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(argv != NULL);
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t ins_num = argv[1]->i;
    check_cond(lr, ins_num >= 1 && ins_num <= INSTRUMENTS_MAX,
            "The Instrument number (%ld)", (long)ins_num);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    Ins_table* table = Song_get_insts(song);
    assert(table != NULL);
    if (Ins_table_get(table, ins_num) != NULL)
    {
        Ins_table_remove(table, ins_num);
    }
    ins_info(lr, song_id, ins_num, NULL);
    return 0;
}


int Listener_ins_get_envelope(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argv;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    Instrument* ins = NULL;
    if (!ins_get(lr, argv[0]->i, argv[1]->i, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL, "The Instrument (%ld)", (long)argv[1]->i);
    return 0; // TODO: This is unfinished...
}


bool ins_get(Listener* lr,
        int32_t song_id,
        int32_t ins_num,
        Instrument** ins)
{
    assert(lr != NULL);
    assert(lr->method_path != NULL);
    assert(ins != NULL);
    check_cond(lr, ins_num >= 1 && ins_num <= INSTRUMENTS_MAX,
            "The Instrument number (%ld)", (long)ins_num);
    get_player(lr, song_id, 'i');
    Song* song = Player_get_song(lr->player_cur);
    Ins_table* table = Song_get_insts(song);
    assert(table != NULL);
    *ins = Ins_table_get(table, ins_num);
    return true;
}


bool ins_info(Listener* lr,
        int32_t song_id,
        int32_t ins_num,
        Instrument* ins)
{
    assert(lr != NULL);
    assert(lr->host != NULL);
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, ins_num);
    if (ins != NULL)
    {
        Generator* gen = Instrument_get_gen(ins, 0);
        assert(gen != NULL);
        lo_message_add_int32(m, Generator_get_type(gen));
        unsigned char mbs[INS_NAME_MAX * 6] = { '\0' };
        wchar_t* src = Instrument_get_name(ins);
        to_utf8_check(lr, mbs, src, INS_NAME_MAX * 6,
                "the name of the Instrument");
        lo_message_add_string(m, (char*)mbs);
        switch (Generator_get_type(gen))
        {
            case GEN_TYPE_PCM:
                if (!ins_info_pcm(lr, m, ins))
                {
                    lo_message_free(m);
                    return false;
                }
                break;
            default:
                break;
        }
    }
    else
    {
        lo_message_add_int32(m, GEN_TYPE_NONE);
        lo_message_add_string(m, "");
    }
    int ret = 0;
    send_msg(lr, "ins_info", m, ret);
    lo_message_free(m);
    if (ret == -1)
    {
        return false;
    }
    return true;
}


