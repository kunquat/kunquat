

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
#include <wchar.h>
#include <string.h>
#include <math.h>

#include "Listener.h"
#include "Listener_note_table.h"
#include "utf8.h"

#include <Song.h>
#include <Note_table.h>
#include <Song_limits.h>


static bool note_table_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index);


static bool note_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index,
		int index);


bool note_mod_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index,
		int index);


int Listener_get_note_table(const char* path,
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
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= NOTE_TABLES_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "notes_sent");
		int ret = lo_send(lr->host, lr->method_path, "ii", player_id, table_index);
		if (ret == -1)
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
		return 0;
	}
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_table_name(const char* path,
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
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	wchar_t name[NOTE_TABLE_NAME_MAX] = { L'\0' };
	unsigned char* src = (unsigned char*)&argv[2]->s;
	if (from_utf8(name, src, NOTE_TABLE_NAME_MAX) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note table name");
	}
	Note_table_set_name(table, name);
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_table_ref_note(const char* path,
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
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	int32_t ref_note_index = argv[2]->i;
	if (ref_note_index < 0 || ref_note_index >= NOTE_TABLE_NOTES)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid reference note index");
		return 0;
	}
	if (!Note_table_set_ref_note(table, ref_note_index))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "si", "No note at index", ref_note_index);
		return 0;
	}
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_table_ref_pitch(const char* path,
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
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	double ref_pitch = argv[2]->d;
	if (!isfinite(ref_pitch) || ref_pitch <= 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid reference pitch");
		return 0;
	}
	Note_table_set_ref_pitch(table, ref_pitch);
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_table_octave_ratio(const char* path,
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
	if (strcmp(types, "iiThh") != 0
			&& strcmp(types, "iiTd") != 0
			&& strcmp(types, "iiFd") != 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "ss", "Invalid type description:", types);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	if (types[2] == 'T')
	{
		if (types[3] == 'h')
		{
			int32_t num = argv[3]->i;
			int32_t den = argv[4]->i;
			if (num <= 0 || den <= 0)
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s", "Invalid octave ratio");
				return 0;
			}
			Real* ratio = Real_init_as_frac(REAL_AUTO, num, den);
			Note_table_set_octave_ratio(table, ratio);
		}
		else
		{
			double rat = argv[3]->d;
			if (!isfinite(rat) || rat <= 0)
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s", "Invalid octave ratio");
				return 0;
			}
			Real* ratio = Real_init_as_double(REAL_AUTO, rat);
			Note_table_set_octave_ratio(table, ratio);
		}
	}
	else
	{
		double cents = argv[3]->d;
		if (!isfinite(cents))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Invalid octave ratio");
			return 0;
		}
		Note_table_set_octave_ratio_cents(table, cents);
	}
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_name(const char* path,
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
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	int32_t note_index = argv[2]->i;
	if (note_index < 0 || note_index >= NOTE_TABLE_NOTES)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid note index");
		return 0;
	}
	wchar_t name[NOTE_TABLE_NOTE_NAME_MAX] = { L'\0' };
	unsigned char* src = (unsigned char*)&argv[3]->s;
	if (from_utf8(name, src, NOTE_TABLE_NOTE_NAME_MAX) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note table name");
	}
	Real* ratio = Real_init(REAL_AUTO);
	if (Note_table_get_note_ratio(table, note_index) != NULL)
	{
		ratio = Note_table_get_note_ratio(table, note_index);
	}
	Note_table_set_note(table, note_index, name, ratio);
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


int Listener_set_note_ratio(const char* path,
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
	if (strcmp(types, "iiiThh") != 0
			&& strcmp(types, "iiiTd") != 0
			&& strcmp(types, "iiiFd") != 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "ss", "Invalid type description:", types);
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
	int32_t table_index = argv[1]->i;
	if (table_index < 0 || table_index >= 16)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Note table index");
		return 0;
	}
	Song* song = player->song;
	Note_table* table = Song_get_notes(song, table_index);
	if (table == NULL)
	{
		if (!Song_create_notes(song, table_index))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for Note table");
			return 0;
		}
		table = Song_get_notes(song, table_index);
		assert(table != NULL);
	}
	int32_t note_index = argv[2]->i;
	if (note_index < 0 || note_index >= NOTE_TABLE_NOTES)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid note index");
		return 0;
	}
	wchar_t* name = NULL;
	wchar_t name_d[NOTE_TABLE_NOTE_NAME_MAX] = { L'\0' };
	if (Note_table_get_note_name(table, note_index) != NULL)
	{
		name = Note_table_get_note_name(table, note_index);
	}
	else
	{
		swprintf(name_d, NOTE_TABLE_NOTE_NAME_MAX - 1, L"(%d)", (int)note_index);
		name = name_d;
	}
	if (types[3] == 'T')
	{
		if (types[4] == 'h')
		{
			int32_t num = argv[4]->i;
			int32_t den = argv[5]->i;
			if (num <= 0 || den <= 0)
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s", "Invalid note ratio");
				return 0;
			}
			Real* ratio = Real_init_as_frac(REAL_AUTO, num, den);
			Note_table_set_note(table, note_index, name, ratio);
		}
		else
		{
			double rat = argv[4]->d;
			if (!isfinite(rat) || rat <= 0)
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s", "Invalid note ratio");
				return 0;
			}
			Real* ratio = Real_init_as_double(REAL_AUTO, rat);
			Note_table_set_note(table, note_index, name, ratio);
		}
	}
	else
	{
		double cents = argv[4]->d;
		if (!isfinite(cents))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Invalid note ratio");
			return 0;
		}
		Note_table_set_note_cents(table, note_index, name, cents);
	}
	if (!note_table_info(lr, player_id, table, table_index))
	{
		return 0;
	}
	return 0;
}


static bool note_table_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index)
{
	assert(lr != NULL);
	assert(table != NULL);
	assert(table_index >= 0);
	assert(table_index < NOTE_TABLES_MAX);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, table_index);
	unsigned char mbs[NOTE_TABLE_NAME_MAX * 6] = { '\0' };
	wchar_t* src = Note_table_get_name(table);
	if (to_utf8(mbs, src, NOTE_TABLE_NAME_MAX * 6) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note table name");
	}
	lo_message_add_string(m, (char*)mbs);
	lo_message_add_int32(m, Note_table_get_note_count(table));
	lo_message_add_int32(m, Note_table_get_note_mod_count(table));
	lo_message_add_int32(m, Note_table_get_ref_note(table));
	lo_message_add_int32(m, Note_table_get_cur_ref_note(table));
	lo_message_add_double(m, Note_table_get_ref_pitch(table));
	Real* ratio = Note_table_get_octave_ratio(table);
	double oct_cents = Note_table_get_octave_ratio_cents(table);
	if (isnan(oct_cents))
	{
		lo_message_add_true(m);
		if (Real_is_frac(ratio))
		{
			lo_message_add_int64(m, Real_get_numerator(ratio));
			lo_message_add_int64(m, Real_get_denominator(ratio));
		}
		else
		{
			lo_message_add_double(m, Real_get_double(ratio));
		}
	}
	else
	{
		lo_message_add_false(m);
		lo_message_add_double(m, oct_cents);
	}
	strcpy(lr->method_path + lr->host_path_len, "note_table_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return false;
	}
	for (int i = 0; i < Note_table_get_note_count(table); ++i)
	{
		if (!note_info(lr, song_id, table, table_index, i))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return false;
		}
	}
	for (int i = 0; i < Note_table_get_note_mod_count(table); ++i)
	{
		if (!note_mod_info(lr, song_id, table, table_index, i))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return false;
		}
	}
	strcpy(lr->method_path + lr->host_path_len, "notes_sent");
	ret = lo_send(lr->host, lr->method_path, "ii", song_id, table_index);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return false;
	}
	return true;
}


static bool note_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index,
		int index)
{
	assert(lr != NULL);
	assert(table != NULL);
	assert(table_index >= 0);
	assert(table_index < NOTE_TABLES_MAX);
	assert(index >= 0);
	assert(index < NOTE_TABLE_NOTES);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, table_index);
	lo_message_add_int32(m, index);
	unsigned char mbs[NOTE_TABLE_NOTE_NAME_MAX * 6] = { '\0' };
	wchar_t* src = Note_table_get_note_name(table, index);
	if (to_utf8(mbs, src, NOTE_TABLE_NOTE_NAME_MAX * 6) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note name");
	}
	lo_message_add_string(m, (char*)mbs);
	bool is_ratio = isnan(Note_table_get_note_cents(table, index));
	if (is_ratio)
	{
		lo_message_add_true(m);
		Real* ratio = Note_table_get_note_ratio(table, index);
		assert(ratio != NULL);
		if (Real_is_frac(ratio))
		{
			lo_message_add_int64(m, Real_get_numerator(ratio));
			lo_message_add_int64(m, Real_get_denominator(ratio));
		}
		else
		{
			lo_message_add_double(m, Real_get_double(ratio));
		}
	}
	else
	{
		lo_message_add_false(m);
		lo_message_add_double(m, Note_table_get_note_cents(table, index));
	}
	if (is_ratio)
	{
		Real* ratio = Note_table_get_cur_note_ratio(table, index);
		assert(ratio != NULL);
		if (Real_is_frac(ratio))
		{
			lo_message_add_int64(m, Real_get_numerator(ratio));
			lo_message_add_int64(m, Real_get_denominator(ratio));
		}
		else
		{
			lo_message_add_double(m, Real_get_double(ratio));
		}
	}
	else
	{
		lo_message_add_double(m, Note_table_get_cur_note_cents(table, index));
	}
	strcpy(lr->method_path + lr->host_path_len, "note_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


bool note_mod_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int table_index,
		int index)
{
	assert(lr != NULL);
	assert(table != NULL);
	assert(table_index >= 0);
	assert(table_index < NOTE_TABLES_MAX);
	assert(index >= 0);
	assert(index < NOTE_TABLE_NOTE_MODS);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, table_index);
	lo_message_add_int32(m, index);
	unsigned char mbs[NOTE_TABLE_NOTE_MOD_NAME_MAX * 6] = { '\0' };
	wchar_t* src = Note_table_get_note_mod_name(table, index);
	if (to_utf8(mbs, src, NOTE_TABLE_NOTE_MOD_NAME_MAX * 6) == EILSEQ)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note modifier name");
	}
	lo_message_add_string(m, (char*)mbs);
	bool is_ratio = isnan(Note_table_get_note_mod_cents(table, index));
	if (is_ratio)
	{
		lo_message_add_true(m);
		Real* ratio = Note_table_get_note_mod_ratio(table, index);
		assert(ratio != NULL);
		if (Real_is_frac(ratio))
		{
			lo_message_add_int64(m, Real_get_numerator(ratio));
			lo_message_add_int64(m, Real_get_denominator(ratio));
		}
		else
		{
			lo_message_add_double(m, Real_get_double(ratio));
		}
	}
	else
	{
		lo_message_add_false(m);
		lo_message_add_double(m, Note_table_get_note_mod_cents(table, index));
	}
	strcpy(lr->method_path + lr->host_path_len, "note_mod_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


