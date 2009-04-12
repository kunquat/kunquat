

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
#include <math.h>

#include <Playlist.h>

#include <xmemory.h>


Playlist* new_Playlist(void)
{
    Playlist* playlist = xalloc(Playlist);
    if (playlist == NULL)
    {
        return NULL;
    }
    playlist->buf_count = 2;
    playlist->buf_size = 128;
    playlist->freq = 44100;
    playlist->voices = 64;
    playlist->first = NULL;
    playlist->reset = true;
    Playlist_reset_stats(playlist);
    return playlist;
}


int32_t Playlist_ins(Playlist* playlist, Song* song)
{
    assert(playlist != NULL);
    assert(song != NULL);
    if (!Song_set_buf_size(song, playlist->buf_size))
    {
        return -1;
    }
    if (!Song_set_buf_count(song, playlist->buf_count))
    {
        return -1;
    }
    Player* player = new_Player(playlist->freq, playlist->voices, song);
    if (player == NULL)
    {
        return -1;
    }
    Playlist_ins_player(playlist, player);
    return Player_get_id(player);
}


Song* Playlist_get(Playlist* playlist, int32_t id)
{
    assert(playlist != NULL);
    Player* player = Playlist_get_player(playlist, id);
    if (player == NULL)
    {
        return NULL;
    }
    assert(Player_get_song(player) != NULL);
    return Player_get_song(player);
}


void Playlist_remove(Playlist* playlist, int32_t id)
{
    assert(playlist != NULL);
    Player* player = Playlist_get_player(playlist, id);
    if (player == NULL)
    {
        return;
    }
    Playlist_remove_player(playlist, player);
    return;
}


void Playlist_ins_player(Playlist* playlist, Player* player)
{
    assert(playlist != NULL);
    assert(player != NULL);
    assert(player->next == NULL);
    assert(player->prev == NULL);
    player->next = playlist->first;
    if (playlist->first != NULL)
    {
        playlist->first->prev = player;
    }
    player->prev = NULL;
    playlist->first = player;
    return;
}


Player* Playlist_get_player(Playlist* playlist, int32_t id)
{
    assert(playlist != NULL);
    Player* cur = playlist->first;
    while (cur != NULL && cur->id != id)
    {
        cur = cur->next;
    }
    return cur;
}


void Playlist_remove_player(Playlist* playlist, Player* player)
{
    assert(playlist != NULL);
    assert(player != NULL);
    Player* next = player->next;
    Player* prev = player->prev;
    if (next != NULL)
    {
        next->prev = prev;
    }
    if (prev != NULL)
    {
        prev->next = next;
    }
    if (player == playlist->first)
    {
        assert(prev == NULL);
        playlist->first = next;
    }
    del_Player(player);
    return;
}


void Playlist_play_song(Playlist* playlist, int32_t id)
{
    assert(playlist != NULL);
    Player* player = playlist->first;
    while (player != NULL)
    {
        if (Player_get_id(player) == id)
        {
            Player_play_song(player);
            break;
        }
        player = player->next;
    }
    return;
}


void Playlist_stop_song(Playlist* playlist, int32_t id)
{
    assert(playlist != NULL);
    Player* player = playlist->first;
    while (player != NULL)
    {
        if (Player_get_id(player) == id)
        {
            Player_stop(player);
            break;
        }
        player = player->next;
    }
    return;
}


bool Playlist_set_buf_count(Playlist* playlist, int count)
{
    assert(playlist != NULL);
    assert(count > 0);
    assert(count <= BUF_COUNT_MAX);
    Player* player = playlist->first;
    while (player != NULL)
    {
        if (!Song_set_buf_count(Player_get_song(player), count))
        {
            return false;
        }
        player = player->next;
    }
    playlist->buf_count = count;
    return true;
}


int Playlist_get_buf_count(Playlist* playlist)
{
    assert(playlist != NULL);
    return playlist->buf_count;
}


bool Playlist_set_buf_size(Playlist* playlist, uint32_t size)
{
    assert(playlist != NULL);
    assert(size > 0);
    Player* player = playlist->first;
    while (player != NULL)
    {
        if (!Song_set_buf_size(Player_get_song(player), size))
        {
            return false;
        }
        player = player->next;
    }
    playlist->buf_size = size;
    return true;
}


uint32_t Playlist_get_buf_size(Playlist* playlist)
{
    assert(playlist != NULL);
    return playlist->buf_size;
}


void Playlist_set_mix_freq(Playlist* playlist, uint32_t freq)
{
    assert(playlist != NULL);
    assert(freq > 0);
    playlist->freq = freq;
    Player* player = playlist->first;
    while (player != NULL)
    {
        Player_set_mix_freq(player, freq);
        player = player->next;
    }
    return;
}


void Playlist_mix(Playlist* playlist, uint32_t nframes, frame_t** bufs)
{
    assert(playlist != NULL);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Player* player = playlist->first;
    int max_buf_count = 0;
    while (player != NULL)
    {
        Playdata* play = Player_get_playdata(player);
        if (!play->mode)
        {
            player = player->next;
            continue;
        }
        assert(play->mode > STOP);
        assert(play->mode < PLAY_LAST);
        uint32_t mixed = Player_mix(player, nframes);
        int buf_count = Song_get_buf_count(player->song);
        if (max_buf_count < buf_count)
        {
            max_buf_count = buf_count;
        }
        frame_t** song_bufs = Song_get_bufs(player->song);
        for (int i = 0; i < buf_count; ++i)
        {
            assert(bufs[i] != NULL);
            for (uint32_t k = 0; k < mixed; ++k)
            {
                bufs[i][k] += song_bufs[i][k];
            }
        }
        player = player->next;
    }
    for (int i = 0; i < max_buf_count; ++i)
    {
        for (uint32_t k = 0; k < nframes; ++k)
        {
            if (playlist->max_values[i] < bufs[i][k])
            {
                playlist->max_values[i] = bufs[i][k];
            }
            if (playlist->min_values[i] > bufs[i][k])
            {
                playlist->min_values[i] = bufs[i][k];
            }
        }
    }
    return;
}


void Playlist_reset_stats(Playlist* playlist)
{
    assert(playlist != NULL);
    if (!playlist->reset)
    {
        return;
    }
    for (int i = 0; i < BUF_COUNT_MAX; ++i)
    {
        playlist->max_values[i] = -INFINITY;
        playlist->min_values[i] = INFINITY;
    }
    Player* player = playlist->first;
    while (player != NULL)
    {
        Playdata* play = Player_get_playdata(player);
        Playdata_reset_stats(play);
        player = player->next;
    }
    playlist->reset = false;
    return;
}


void Playlist_schedule_reset(Playlist* playlist)
{
    assert(playlist != NULL);
    playlist->reset = true;
}


void del_Playlist(Playlist* playlist)
{
    assert(playlist != NULL);
    Player* target = playlist->first;
    playlist->first = NULL;
    while (target != NULL)
    {
        Player* next = target->next;
        del_Player(target);
        target = next;
    }
    xfree(playlist);
    return;
}


