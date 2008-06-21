

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
#include <math.h>

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
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	get_player(lr, player_id, types[0]);
	Player_stop(lr->player_cur);
	player_state(lr, player_id, "stop");
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
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	get_player(lr, player_id, types[0]);
	if (lr->driver_id == -1)
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "No active driver");
		int ret = 0;
		send_msg(lr, "notify", m, ret);
		lo_message_free(m);
		return 0;
	}
	Player_play_song(lr->player_cur);
	player_state(lr, player_id, "song");
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
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	int32_t player_id = argv[0]->i;
	get_player(lr, player_id, types[0]);
	if (lr->driver_id == -1)
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "No active driver");
		int ret = 0;
		send_msg(lr, "notify", m, ret);
		lo_message_free(m);
		return 0;
	}
	int32_t subsong = argv[1]->i;
	check_cond(lr, subsong >= 0 && subsong < SUBSONGS_MAX,
			"The subsong number (%ld)", (long)subsong);
	Player_play_subsong(lr->player_cur, subsong);
	player_state(lr, player_id, "song");
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
	get_player(lr, player_id, types[0]);
	if (lr->driver_id == -1)
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "No active driver");
		int ret = 0;
		send_msg(lr, "notify", m, ret);
		lo_message_free(m);
		return 0;
	}
	int32_t pat_num = argv[1]->i;
	check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
			"The Pattern number (%ld)", (long)pat_num);
	double tempo = argv[2]->d;
	check_cond(lr, isfinite(tempo) && tempo > 0,
			"The tempo (%f)", tempo);
	Player_play_pattern(lr->player_cur, pat_num, tempo);
	player_state(lr, player_id, "pattern");
	return 0;
}


int Listener_play_event(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)msg;
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	check_cond(lr, strncmp("iii", types, 3) == 0,
			"The argument type list (%s)", types);
	if (lr->driver_id == -1)
	{
		return 0;
	}
	int32_t player_id = argv[0]->i;
	get_player(lr, player_id, types[0]);
	Playdata* play = Player_get_playdata(lr->player_cur);
	assert(play != NULL);
	int32_t ch = argv[1]->i;
	check_cond(lr, ch >= 1 && ch <= 64,
			"The Channel number (%ld)", (long)ch);
	--ch;
	int32_t event_type = argv[2]->i;
	check_cond(lr, EVENT_TYPE_IS_INS(event_type),
			"The Event type (%ld)", (long)event_type);
	char* type_desc = Event_type_get_field_types(event_type);
	check_cond(lr, type_desc != NULL,
			"The Event type (%ld) is unused -- the type description",
			(long)event_type);
	int field_count = strlen(type_desc);
	int num_args = argc - 3;
	check_cond(lr, num_args == field_count,
			"The number of Event parameters (%d)", num_args);
	Event* event = play->channels[ch]->single;
	Event_reset(event, event_type);
	for (int i = 0; i < field_count; ++i)
	{
		char type = types[3 + i];
		if (type_desc[i] == 'i')
		{
			check_cond(lr, type == 'h',
					"The type of the Event field #%d (%c)", i, type);
			if (!Event_set_int(event, i, argv[3 + i]->h))
			{
				lo_message m = new_msg();
				lo_message_add_string(m, "Invalid value of the field");
				lo_message_add_int32(m, i);
				lo_message_add_int32(m, argv[3 + i]->h);
				int ret = 0;
				send_msg(lr, "error", m, ret);
				lo_message_free(m);
				return 0;
			}
		}
		else if (type_desc[i] == 'f')
		{
			check_cond(lr, type == 'd',
					"The type of the Event field #%d (%c)", i, type);
			if (!Event_set_float(event, i, argv[3 + i]->d))
			{
				lo_message m = new_msg();
				lo_message_add_string(m, "Invalid value of the field");
				lo_message_add_int32(m, i);
				lo_message_add_double(m, argv[3 + i]->d);
				int ret = 0;
				send_msg(lr, "error", m, ret);
				lo_message_free(m);
				return 0;
			}
		}
	}
	Event_set_pos(event, Reltime_init(RELTIME_AUTO));
	Player_play_event(lr->player_cur);
	player_state(lr, player_id, "event");
	return 0;
}


static bool player_state(Listener* lr,
		int32_t song_id,
		char* state)
{
	assert(lr != NULL);
	assert(state != NULL);
	lo_message m = new_msg();
	lo_message_add_int32(m, song_id);
	lo_message_add_string(m, state);
	int ret = 0;
	send_msg(lr, "player_state", m, ret);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


