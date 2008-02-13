

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
#include <stdbool.h>

#include "Listener.h"
#include "Listener_ins.h"

#include <Song.h>
#include <Ins_table.h>
#include <Instrument.h>


static bool ins_info(Listener* l,
		int32_t song_id,
		int32_t ins_num,
		Instrument* ins);


int Listener_new_ins(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)msg;
	assert(argc == 3);
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	if (argv[1]->i < 1 || argv[1]->i > 255)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "si", "Invalid Instrument number:", argv[1]->i);
		return 0;
	}
	int32_t player_id = argv[0]->i;
	Player* player = l->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(l->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Instrument* ins = new_Instrument(INS_TYPE_SINE,
			Song_get_bufs(song),
			Song_get_buf_size(song),
			32); // XXX: get event count from the configuration
	if (ins == NULL)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "s", "Couldn't allocate memory");
		return 0;
	}
	Ins_table* table = Song_get_insts(song);
	assert(table != NULL);
	if (!Ins_table_set(table, argv[1]->i, ins))
	{
		del_Instrument(ins);
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "s", "Couldn't allocate memory");
		return 0;
	}
	if (!ins_info(l, player_id, argv[1]->i, ins))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


#if 0
int Listener_ins_set_name(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
}
#endif


int Listener_del_ins(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)msg;
	assert(argc == 2);
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	if (argv[1]->i < 1 || argv[1]->i > 255)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "si", "Invalid Instrument number:", argv[1]->i);
		return 0;
	}
	int32_t player_id = argv[0]->i;
	Player* player = l->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(l->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Ins_table* table = Song_get_insts(song);
	assert(table != NULL);
	if (Ins_table_get(table, argv[1]->i) != NULL)
	{
		Ins_table_remove(table, argv[1]->i);
	}
	if (!ins_info(l, player_id, argv[1]->i, NULL))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


static bool ins_info(Listener* l,
		int32_t song_id,
		int32_t ins_num,
		Instrument* ins)
{
	assert(l != NULL);
	assert(l->host != NULL);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, ins_num);
	if (ins != NULL)
	{
		lo_message_add_int32(m, Instrument_get_type(ins));
		lo_message_add_string(m, ""); // TODO: real name
	}
	else
	{
		lo_message_add_int32(m, INS_TYPE_NONE);
	}
	strcpy(l->method_path + l->host_path_len, "ins_info");
	int ret = lo_send_message(l->host, l->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


