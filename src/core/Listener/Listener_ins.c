

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
#include <stdio.h>

#include "Listener.h"
#include "Listener_ins.h"
#include "utf8.h"

#include <Song.h>
#include <Ins_table.h>
#include <Instrument.h>
#include <Song_limits.h>


/**
 * Finds the Instrument based on given Song ID and Instrument number.
 *
 * If the requested Song exists, it will be made current Song.
 *
 * \param l         The Listener -- must not be \c NULL.
 * \param song_id   The Song ID.
 * \param ins_num   The Instrument number.
 * \param ins       The location to which the Instrument will be stored
 *                  -- must not be \c NULL.
 *
 * \return   \c true if the Song exists and Instrument number is valid,
 *           otherwise \c false. Note that \c true will be returned if the
 *           search parameters are valid but no Instrument is found.
 */
static bool ins_get(Listener* l,
		int32_t song_id,
		int32_t ins_num,
		Instrument** ins);


/**
 * Sends the ins_info message.
 *
 * \param l         The Listener -- must not be \c NULL.
 * \param song_id   The Song ID.
 * \param ins_num   The Instrument number.
 * \param ins       The Instrument located in \a song_id, \a ins_num.
 *
 * \return   \c true if the message was sent successfully, otherwise \c false.
 */
static bool ins_info(Listener* l,
		int32_t song_id,
		int32_t ins_num,
		Instrument* ins);


int Listener_get_insts(const char* path,
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
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
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
	for (int i = 1; i <= INSTRUMENTS_MAX; ++i)
	{
		Instrument* ins = Ins_table_get(table, i);
		if (ins != NULL)
		{
			if (!ins_info(l, player_id, i, ins))
			{
				fprintf(stderr, "Couldn't send the response message\n");
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
	if (argv[1]->i < 1 || argv[1]->i > INSTRUMENTS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "si", "Invalid Instrument number:", argv[1]->i);
		return 0;
	}
	if (argv[2]->i < INS_TYPE_NONE || argv[2]->i >= INS_TYPE_LAST)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "si", "Invalid Instrument type:", argv[2]->i);
		return 0;
	}
	if (argv[2]->i > INS_TYPE_SINE)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Only debug and sine instruments supported");
		return 0;
	}
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
	Ins_table* table = Song_get_insts(song);
	assert(table != NULL);
	Instrument* ins = NULL;
	if (argv[2]->i != INS_TYPE_NONE)
	{
		ins = new_Instrument(argv[2]->i,
				Song_get_bufs(song),
				Song_get_buf_size(song),
				32); // XXX: get event count from the configuration
		if (ins == NULL)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory");
			return 0;
		}
		Instrument_set_note_table(ins, Song_get_active_notes(song));
		if (!Ins_table_set(table, argv[1]->i, ins))
		{
			del_Instrument(ins);
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory");
			return 0;
		}
	}
	else
	{
		Ins_table_remove(table, argv[1]->i);
	}
	if (!ins_info(lr, player_id, argv[1]->i, ins))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
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
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	Instrument* ins = NULL;
	if (!ins_get(l, argv[0]->i, argv[1]->i, &ins))
	{
		return 0;
	}
	if (ins != NULL)
	{
		wchar_t name[INS_NAME_MAX] = { L'\0' };
		unsigned char* src = (unsigned char*)&argv[2]->s;
		if (from_utf8(name, src, INS_NAME_MAX) == EILSEQ)
		{
			strcpy(l->method_path + l->host_path_len, "error");
			lo_send(l->host, l->method_path, "s",
					"Illegal character sequence in the Instrument name");
		}
		Instrument_set_name(ins, name);
	}
	if (!ins_info(l, l->player_cur->id, argv[1]->i, ins))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
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
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	if (argv[1]->i < 1 || argv[1]->i > INSTRUMENTS_MAX)
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


static bool ins_get(Listener* l,
		int32_t song_id,
		int32_t ins_num,
		Instrument** ins)
{
	assert(l != NULL);
	assert(l->method_path != NULL);
	assert(ins != NULL);
	if (ins_num < 1 || ins_num > INSTRUMENTS_MAX)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "si", "Invalid Instrument number:", ins_num);
		return false;
	}
	int32_t player_id = song_id;
	Player* player = l->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(l->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host, l->method_path, "s", "Song doesn't exist");
		return false;
	}
	l->player_cur = player;
	Song* song = player->song;
	Ins_table* table = Song_get_insts(song);
	assert(table != NULL);
	*ins = Ins_table_get(table, ins_num);
	return true;
}


static bool ins_info(Listener* lr,
		int32_t song_id,
		int32_t ins_num,
		Instrument* ins)
{
	assert(lr != NULL);
	assert(lr->host != NULL);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, ins_num);
	if (ins != NULL)
	{
		lo_message_add_int32(m, Instrument_get_type(ins));
		unsigned char mbs[INS_NAME_MAX * 6] = { '\0' };
		wchar_t* src = Instrument_get_name(ins);
		if (to_utf8(mbs, src, INS_NAME_MAX * 6) == EILSEQ)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Illegal character sequence in the Instrument name");
		}
		lo_message_add_string(m, (char*)mbs);
	}
	else
	{
		lo_message_add_int32(m, INS_TYPE_NONE);
		lo_message_add_string(m, "");
	}
	strcpy(lr->method_path + lr->host_path_len, "ins_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


