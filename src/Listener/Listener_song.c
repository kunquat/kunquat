

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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "Listener.h"
#include "Listener_song.h"
#include "utf8.h"


static bool song_info(Listener* lr,
        int32_t song_id,
        Song* song);


static bool subsong_info(Listener* lr,
        int32_t song_id,
        int32_t subsong,
        Song* song);


int Listener_new_song(const char* path,
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
    Player* player = NULL;
    Song* song = new_Song(Playlist_get_buf_count(lr->playlist),
            Playlist_get_buf_size(lr->playlist),
            16); // TODO: get from the Listener
    if (song != NULL)
    {
        player = new_Player(lr->freq, lr->voice_count, song); // TODO: freq
    }
    if (player == NULL)
    {
        if (song != NULL)
        {
            del_Song(song);
        }
        send_memory_fail(lr, "the new Song");
    }
    assert(song != NULL);
    Playlist_ins_player(lr->playlist, player);
    lr->player_cur = player;
    lo_message m = new_msg();
    lo_message_add_int32(m, player->id);
    int ret = 0;
    send_msg(lr, "new_song", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_get_songs(const char* path,
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
    lo_message m = new_msg();
    Player* player = lr->playlist->first;
    while (player != NULL)
    {
        lo_message_add_int32(m, player->id);
        player = player->next;
    }
    int ret = 0;
    send_msg(lr, "songs", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_get_song_info(const char* path,
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
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    if (!song_info(lr, song_id, song))
    {
        return 0;
    }
    for (int32_t i = 0; i < SUBSONGS_MAX; ++i)
    {
        if (!subsong_info(lr, song_id, i, song))
        {
            return 0;
        }
    }
    return 0;
}


int Listener_set_song_title(const char* path,
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
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    wchar_t title[SONG_NAME_MAX] = { L'\0' };
    unsigned char* src = (unsigned char*)&argv[1]->s;
    check_cond(lr, src != NULL, "The song title (%s)", src);
    from_utf8_check(lr, title, src, SONG_NAME_MAX, "the Song title");
    Song* song = Player_get_song(lr->player_cur);
    Song_set_name(song, title);
    song_info(lr, song_id, song);
    return 0;
}


int Listener_set_mix_vol(const char* path,
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
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    double mix_vol = argv[1]->d;
    check_cond(lr, mix_vol <= 0,
            "The mixing volume (%f)\n", mix_vol);
    Song_set_mix_vol(song, mix_vol);
    song_info(lr, song_id, song);
    return 0;
}


int Listener_set_subsong_tempo(const char* path,
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
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    int32_t subsong = argv[1]->i;
    check_cond(lr, subsong >= 0 && subsong < SUBSONGS_MAX,
            "The subsong number (%ld)", (long)subsong);
    double tempo = argv[2]->d;
    check_cond(lr, isfinite(tempo) && tempo > 0,
            "The tempo (%f)", tempo);
    Song_set_tempo(song, subsong, tempo);
    subsong_info(lr, song_id, subsong, song);
    return 0;
}


int Listener_set_subsong_global_vol(const char* path,
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
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    int32_t subsong = argv[1]->i;
    check_cond(lr, subsong >= 0 && subsong < SUBSONGS_MAX,
            "The subsong number (%ld)", (long)subsong);
    double global_vol = argv[2]->d;
    check_cond(lr, isfinite(global_vol) || isinf(global_vol) == 1,
            "The global volume (%f)", global_vol);
    Song_set_global_vol(song, subsong, global_vol);
    subsong_info(lr, song_id, subsong, song);
    return 0;
}


int Listener_del_song(const char* path,
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
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Player* target = lr->player_cur;
    assert(target->id == song_id);
    Playlist_remove_player(lr->playlist, target);
    lr->player_cur = lr->playlist->first;
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    int ret = 0;
    send_msg(lr, "del_song", m, ret);
    lo_message_free(m);
    return 0;
}


static bool song_info(Listener* lr,
        int32_t song_id,
        Song* song)
{
    assert(lr != NULL);
    assert(song != NULL);
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    unsigned char mbs[SONG_NAME_MAX * 6] = { '\0' };
    wchar_t* src = Song_get_name(song);
    to_utf8_check(lr, mbs, src, SONG_NAME_MAX * 6, "the Song title");
    lo_message_add_string(m, (char*)mbs);
    lo_message_add_double(m, song->mix_vol);
    lo_message_add_int32(m, song->init_subsong);
    int ret = 0;
    send_msg(lr, "song_info", m, ret);
    lo_message_free(m);
    if (ret == -1)
    {
        return false;
    }
    return true;
}


static bool subsong_info(Listener* lr,
        int32_t song_id,
        int32_t subsong,
        Song* song)
{
    assert(lr != NULL);
    assert(subsong >= 0);
    assert(subsong < SUBSONGS_MAX);
    assert(song != NULL);
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, subsong);
    double tempo = Song_get_tempo(song, subsong);
    double global_vol = Song_get_global_vol(song, subsong);
    lo_message_add_double(m, tempo);
    lo_message_add_double(m, global_vol);
    int ret = 0;
    send_msg(lr, "subsong_info", m, ret);
    lo_message_free(m);
    if (ret == -1)
    {
        return false;
    }
    return true;
}


