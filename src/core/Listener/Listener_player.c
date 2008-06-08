

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
#include <stdio.h>

#include "Listener.h"
#include "Listener_player.h"

#include <Player.h>
#include <Playdata.h>


static bool player_state(Listener* lr,
		int32_t song_id,
		char* state);


int Listener_stop(const char* path,
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
	Player* player = lr->playlist->first;
	while (player != NULL)
	{
		Player_stop(player);
		player_state(lr, player->id, "stop");
		player = player->next;
	}
	return 0;
}


int Listener_stop_song(const char* path,
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
	Player_stop(player);
	player_state(lr, player->id, "stop");
	return 0;
}


int Listener_play_song(const char* path,
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
	if (lr->driver_id == -1)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "No active driver");
		return 0;
	}
	Player_play_song(player);
	player_state(lr, player->id, "song");
	return 0;
}


int Listener_play_subsong(const char* path,
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
	if (lr->driver_id == -1)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "No active driver");
		return 0;
	}
	if (argv[0]->i < 0 || argv[0]->i >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	Player_play_subsong(player, argv[0]->i);
	player_state(lr, player->id, "song");
	return 0;
}


int Listener_play_pattern(const char* path,
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
	if (lr->driver_id == -1)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "No active driver");
		return 0;
	}
	if (argv[1]->i < 0 || argv[1]->i > PATTERNS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Pattern number");
		return 0;
	}
	Player_play_pattern(player, argv[1]->i);
	player_state(lr, player->id, "pattern");
	return 0;
}


static bool player_state(Listener* lr,
		int32_t song_id,
		char* state)
{
	assert(lr != NULL);
	assert(state != NULL);
	strcpy(lr->method_path + lr->host_path_len, "player_state");
	if (lo_send(lr->host, lr->method_path, "is", song_id, state) == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return false;
	}
	return true;
}


