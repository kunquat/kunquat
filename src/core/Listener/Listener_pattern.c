

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
#include "Listener_pattern.h"

#include <Song.h>
#include <Pattern.h>
#include <Reltime.h>


/**
 * Sends the event_info message.
 *
 * \param lr         The Listener -- must not be \c NULL.
 * \param song_id    The Song ID.
 * \param pat_num    The Pattern number.
 * \param ch_num     The Channel number.
 * \param index      The 0-based order of the Event in this location.
 * \param event      The Event -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool event_info(Listener* lr,
		int32_t song_id,
		int32_t pat_num,
		int32_t ch_num,
		int32_t index,
		Event* event);


/**
 * Sends the pat_info message (also calls event_info if needed).
 *
 * \param lr        The Listener -- must not be \c NULL.
 * \param song_id   The Song ID.
 * \param pat_num   The Pattern number.
 * \param pat       The Pattern.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool pat_info(Listener* lr,
		int32_t song_id,
		int32_t pat_num,
		Pattern* pat);


int Listener_get_pattern(const char* path,
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
	int32_t pat_num = argv[1]->i;
	if (pat_num < 0 || pat_num >= PATTERNS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Pattern number");
		return 0;
	}
	Song* song = player->song;
	Pat_table* table = Song_get_pats(song);
	Pattern* pat = Pat_table_get(table, pat_num);
	if (!pat_info(lr, player_id, pat_num, pat))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	strcpy(lr->method_path + lr->host_path_len, "events_sent");
	int ret = lo_send(lr->host, lr->method_path, "ii", player->id, pat_num);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


#if 0
int Listener_get_pats(const char* path,
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
	Pat_table* table = Song_get_pats(song);
	for (int i = 0; i < PATTERNS_MAX; ++i)
	{
		Pattern* pat = Pat_table_get(table, i);
		if (pat == NULL)
		{
			continue;
		}
		for (int k = -1; k < 64; ++k)
		{
			Column* col = Pattern_global(pat);
			if (k > -1)
			{
				col = Pattern_col(pat, k);
			}
			Event* event = Column_get(col, Reltime_init(RELTIME_AUTO));
			int index = 0;
			Reltime* prev_time = Reltime_set(RELTIME_AUTO, INT64_MIN, 0);
			Reltime* time = RELTIME_AUTO;
			while (event != NULL)
			{
				Reltime_copy(time, Event_pos(event));
				if (Reltime_cmp(prev_time, time) == 0)
				{
					++index;
				}
				else
				{
					index = 0;
				}
				Reltime_copy(prev_time, time);
				if (!event_info(lr, player_id, i, k, index, event))
				{
					fprintf(stderr, "Couldn't send the response message\n");
					return 0;
				}
				event = Column_get_next(col);
			}
		}
	}
	return 0;
}
#endif


/*
int Listener_new_pat(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
}


int Listener_del_pat(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
}
*/

/*
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. A new Pattern will be created if necessary.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c i   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c s   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */

int Listener_pat_ins_event(const char* path,
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
	if (argc < 7)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Not enough arguments (at least 7 needed)");
		return 0;
	}
	if (types[0] != 'i')
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Song ID is not an integer");
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
	Pat_table* table = Song_get_pats(song);
	int32_t pat_num = argv[1]->i;
	if (types[1] != 'i' || pat_num < 0 || pat_num >= 1024)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Pattern number");
		return 0;
	}
	Pattern* pat = Pat_table_get(table, pat_num);
	if (pat == NULL)
	{
		pat = new_Pattern();
		if (pat == NULL)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Couldn't allocate memory for new Pattern");
			return 0;
		}
		if (!Pat_table_set(table, pat_num, pat))
		{
			del_Pattern(pat);
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Couldn't allocate memory for new Pattern");
			return 0;
		}
	}
	int32_t col_num = argv[2]->i;
	if (types[2] != 'i' || col_num < 0 || col_num > 64)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Column number");
		return 0;
	}
	Column* col = Pattern_global(pat);
	if (col_num > 0)
	{
		col = Pattern_col(pat, col_num - 1);
	}
	int64_t beats = argv[3]->h;
	if (types[3] != 'h')
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid beat number");
		return 0;
	}
	int32_t part = argv[4]->i;
	if (types[4] != 'i' || part < 0 || part >= RELTIME_FULL_PART)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event position number");
		return 0;
	}
	Reltime* pos = Reltime_set(RELTIME_AUTO, beats, part);
	int32_t event_order = argv[5]->i;
	if (types[5] != 'i' || event_order < 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event order index");
		return 0;
	}
	int32_t event_type = argv[6]->i;
	if (!EVENT_TYPE_IS_VALID(event_type))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event type");
		return 0;
	}
	union { int64_t i; double d; } event_values[EVENT_FIELDS];
	if (col_num == 0)
	{
		switch (event_type)
		{
			case EVENT_TYPE_NOTE_ON:
			case EVENT_TYPE_NOTE_OFF:
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s",
						"Event type unsupported by global event channel");
				return 0;
			default:
				break;
		}
	}
	else
	{
		switch (event_type)
		{
			case EVENT_TYPE_NOTE_ON:
				if (argc < 11)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Not enough Event fields for Note On");
					return 0;
				}
				event_values[0].i = argv[7]->h;
				if (types[7] != 'h' || event_values[0].i < 0
						|| event_values[0].i >= NOTE_TABLE_NOTES)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid note value for Note On");
					return 0;
				}
				event_values[1].i = argv[8]->h;
				if (types[8] != 'h' || event_values[1].i < -1
						|| event_values[1].i >= NOTE_TABLE_NOTE_MODS)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid note modifier for Note On");
					return 0;
				}
				event_values[2].i = argv[9]->h;
				if (types[9] != 'h' || event_values[2].i < NOTE_TABLE_OCTAVE_FIRST
						|| event_values[2].i > NOTE_TABLE_OCTAVE_LAST)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid octave for Note On");
					return 0;
				}
				event_values[3].i = argv[10]->h;
				if (types[10] != 'h' || event_values[3].i <= 0
						|| event_values[3].i > INSTRUMENTS_MAX)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid Instrument number for Note On");
					return 0;
				}
				break;
			case EVENT_TYPE_NOTE_OFF:
				break;
			default:
				break;
		}
	}
	Event* event = new_Event(pos, event_type);
	if (event == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Couldn't allocate memory for new Event");
		return 0;
	}
	switch (event_type)
	{
		case EVENT_TYPE_NOTE_ON:
			for (int i = 0; i < 4; ++i)
			{
				bool set_val = Event_set_int(event, i, event_values[i].i);
				assert(set_val);
			}
			break;
		case EVENT_TYPE_NOTE_OFF:
			break;
		default:
			break;
	}
	if (!Column_ins(col, event))
	{
		del_Event(event);
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Couldn't allocate memory for new Event");
		return 0;
	}
	if (Column_move(col, event, event_order))
	{
		if (!pat_info(lr, player_id, pat_num, pat))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
	}
	else if (!event_info(lr, player_id, pat_num, col_num, event_order, event))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	strcpy(lr->method_path + lr->host_path_len, "events_sent");
	int ret = lo_send(lr->host, lr->method_path, "ii", player->id, pat_num);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


static bool event_info(Listener* lr,
		int32_t song_id,
		int32_t pat_num,
		int32_t ch_num,
		int32_t index,
		Event* event)
{
	assert(lr != NULL);
	assert(pat_num >= 0);
	assert(ch_num >= -1);
	assert(ch_num < 64);
	assert(index >= 0);
	assert(event != NULL);
	Reltime* pos = Event_pos(event);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, pat_num);
	lo_message_add_int32(m, ch_num);
	lo_message_add_int64(m, pos->beats);
	lo_message_add_int32(m, pos->part);
	lo_message_add_int32(m, index);
	lo_message_add_int32(m, event->type);
	bool got_val = false;
	int64_t val = 0;
	switch (event->type)
	{
		case EVENT_TYPE_NOTE_ON:
			for (int i = 0; i < 4; ++i)
			{
				got_val = Event_int(event, i, &val);
				assert(got_val);
				lo_message_add_int64(m, val);
			}
			break;
		case EVENT_TYPE_NOTE_OFF:
			break;
		default:
			break;
	}
	strcpy(lr->method_path + lr->host_path_len, "event_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


static bool pat_info(Listener* lr,
		int32_t song_id,
		int32_t pat_num,
		Pattern* pat)
{
	if (pat != NULL)
	{
		Reltime* pat_len = Pattern_get_length(pat);
		strcpy(lr->method_path + lr->host_path_len, "pat_info");
		int ret = lo_send(lr->host, lr->method_path, "iihi",
				song_id,
				pat_num,
				pat_len->beats,
				pat_len->part);
		if (ret == -1)
		{
			return false;
		}
		for (int i = -1; i < 64; ++i)
		{
			Column* col = Pattern_global(pat);
			if (i > -1)
			{
				col = Pattern_col(pat, i);
			}
			Event* event = Column_get(col, Reltime_init(RELTIME_AUTO));
			int index = 0;
			Reltime* prev_time = Reltime_set(RELTIME_AUTO, INT64_MIN, 0);
			Reltime* time = RELTIME_AUTO;
			while (event != NULL)
			{
				Reltime_copy(time, Event_pos(event));
				if (Reltime_cmp(prev_time, time) == 0)
				{
					++index;
				}
				else
				{
					index = 0;
				}
				Reltime_copy(prev_time, time);
				if (!event_info(lr, song_id, pat_num, i + 1, index, event))
				{
					return false;
				}
				event = Column_get_next(col);
			}
		}
	}
	else
	{
		strcpy(lr->method_path + lr->host_path_len, "pat_info");
		int ret = lo_send(lr->host, lr->method_path, "iihi",
				song_id,
				pat_num,
				(int64_t)16, (int32_t)0);
		if (ret == -1)
		{
			return false;
		}
	}
	return true;
}


