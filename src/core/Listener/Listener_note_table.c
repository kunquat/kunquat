

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

#include <Song.h>
#include <Note_table.h>


bool note_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int index);


bool note_mod_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
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
	Song* song = player->song;
	Note_table* table = Song_get_notes(song);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, player_id);
	char mbs[NOTE_TABLE_NAME_MAX * 6] = { '\0' };
	const wchar_t* src = Note_table_get_name(table);
	if (wcsrtombs(mbs, &src, NOTE_TABLE_NAME_MAX * 6, NULL) == (size_t)(-1))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note table name");
	}
	lo_message_add_string(m, mbs);
	lo_message_add_int32(m, Note_table_get_note_count(table));
	lo_message_add_int32(m, Note_table_get_note_mod_count(table));
	lo_message_add_int32(m, Note_table_get_ref_note(table));
	lo_message_add_int32(m, Note_table_get_cur_ref_note(table));
	lo_message_add_double(m, Note_table_get_ref_pitch(table));
	Real* ratio = Note_table_get_octave_ratio(table);
	if (Real_is_frac(ratio))
	{
		lo_message_add_int64(m, Real_get_numerator(ratio));
		lo_message_add_int64(m, Real_get_denominator(ratio));
	}
	else
	{
		lo_message_add_double(m, Real_get_double(ratio));
	}
	strcpy(lr->method_path + lr->host_path_len, "note_table_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	for (int i = 0; i < Note_table_get_note_count(table); ++i)
	{
		if (!note_info(lr, player_id, table, i))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
	}
	for (int i = 0; i < Note_table_get_note_mod_count(table); ++i)
	{
		if (!note_mod_info(lr, player_id, table, i))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
	}
	strcpy(lr->method_path + lr->host_path_len, "notes_sent");
	ret = lo_send(lr->host, lr->method_path, "i", player->id);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


bool note_info(Listener* lr,
		int32_t song_id,
		Note_table* table,
		int index)
{
	assert(lr != NULL);
	assert(table != NULL);
	assert(index >= 0);
	assert(index < NOTE_TABLE_NOTES);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, index);
	char mbs[NOTE_TABLE_NOTE_NAME_MAX * 6] = { '\0' };
	const wchar_t* src = Note_table_get_note_name(table, index);
	if (wcsrtombs(mbs, &src, NOTE_TABLE_NOTE_NAME_MAX * 6, NULL) == (size_t)(-1))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note name");
	}
	lo_message_add_string(m, mbs);
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
		int index)
{
	assert(lr != NULL);
	assert(table != NULL);
	assert(index >= 0);
	assert(index < NOTE_TABLE_NOTE_MODS);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, index);
	char mbs[NOTE_TABLE_NOTE_MOD_NAME_MAX * 6] = { '\0' };
	const wchar_t* src = Note_table_get_note_mod_name(table, index);
	if (wcsrtombs(mbs, &src, NOTE_TABLE_NOTE_MOD_NAME_MAX * 6, NULL) == (size_t)(-1))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Illegal character sequence in the Note modifier name");
	}
	lo_message_add_string(m, mbs);
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


