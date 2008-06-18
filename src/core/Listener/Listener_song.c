

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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
	Listener* l = user_data;
	Player* player = NULL;
	Song* song = new_Song(2, 128, 16); // TODO: get params from relevant parts of the Listener
	if (song != NULL)
	{
		player = new_Player(l->freq, l->voice_count, song); // TODO: freq
	}
	if (player == NULL)
	{
		if (song != NULL)
		{
			del_Song(song);
		}
		strcpy(l->method_path + l->host_path_len, "new_song");
		lo_send(l->host, l->method_path, "s", "Couldn't allocate memory");
		return 0;
	}
	assert(song != NULL);
	Playlist_ins(l->playlist, player);
	l->player_cur = player;
	strcpy(l->method_path + l->host_path_len, "new_song");
	int ret = lo_send(l->host, l->method_path, "i", player->id);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
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
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	lo_message m = lo_message_new();
	if (m == NULL)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	Player* player = l->playlist->first;
	while (player != NULL)
	{
		lo_message_add_int32(m, player->id);
		player = player->next;
	}
	strcpy(l->method_path + l->host_path_len, "songs");
	int ret = lo_send_message(l->host, l->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
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
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	if (!song_info(lr, player_id, song))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	for (int32_t i = 0; i < SUBSONGS_MAX; ++i)
	{
		if (!subsong_info(lr, player_id, i, song))
		{
			fprintf(stderr, "Couldn't send the response message\n");
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
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	wchar_t title[SONG_NAME_MAX] = { L'\0' };
	unsigned char* src = (unsigned char*)&argv[1]->s;
	if (src == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"NULL string passed as the new title");
		return 0;
	}
	if (from_utf8(title, src, SONG_NAME_MAX) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Song title");
	}
	Song* song = player->song;
	Song_set_name(song, title);
	if (!song_info(lr, player_id, song))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
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
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	int32_t subsong = argv[1]->i;
	if (subsong < 0 || subsong >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	double tempo = argv[2]->d;
	if (!isfinite(tempo) || tempo <= 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid tempo");
		return 0;
	}
	Song_set_tempo(song, subsong, tempo);
	if (!subsong_info(lr, player_id, subsong, song))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
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
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	int32_t subsong = argv[1]->i;
	if (subsong < 0 || subsong >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	double global_vol = argv[2]->d;
	if (!isfinite(global_vol) && isinf(global_vol) != -1)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid global volume");
		return 0;
	}
	Song_set_global_vol(song, subsong, global_vol);
	if (!subsong_info(lr, player_id, subsong, song))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
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
	Listener* l = user_data;
	int32_t player_id = argv[0]->i;
	Player* target = l->player_cur;
	if (target == NULL || target->id != player_id)
	{
		target = Playlist_get(l->playlist, player_id);
	}
	if (target == NULL)
	{
		strcpy(l->method_path + l->host_path_len, "del_song");
		lo_send(l->host, l->method_path, "");
		return 0;
	}
	assert(target->id == player_id);
	Playlist_remove(l->playlist, target);
	l->player_cur = l->playlist->first;
	strcpy(l->method_path + l->host_path_len, "del_song");
	int ret = lo_send(l->host, l->method_path, "i", player_id);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	return 0;
}


static bool song_info(Listener* lr,
		int32_t song_id,
		Song* song)
{
	assert(lr != NULL);
	assert(song != NULL);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	unsigned char mbs[SONG_NAME_MAX * 6] = { '\0' };
	wchar_t* src = Song_get_name(song);
	if (to_utf8(mbs, src, SONG_NAME_MAX * 6) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Song title");
	}
	lo_message_add_string(m, (char*)mbs);
	lo_message_add_double(m, song->mix_vol);
	lo_message_add_int32(m, song->init_subsong);
	strcpy(lr->method_path + lr->host_path_len, "song_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
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
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, subsong);
	double tempo = Song_get_tempo(song, subsong);
	double global_vol = Song_get_global_vol(song, subsong);
	lo_message_add_double(m, tempo);
	lo_message_add_double(m, global_vol);
	strcpy(lr->method_path + lr->host_path_len, "subsong_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


