

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
#include <string.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_pattern.h"

#include <Song.h>
#include <Pattern.h>
#include <Reltime.h>
#include <Event.h>


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


/**
 * Checks that the given Event reference is valid.
 *
 * \param lr           The Listener -- must not be \c NULL.
 * \param types        The OSC type description -- must not be \c NULL.
 * \param argv         The OSC arguments -- must not be \c NULL.
 * \param song         Location for the Song to be set -- must not be \c NULL.
 * \param pat          Location for the Pattern to be set -- must not be
 *                     \c NULL.
 * \param expect_pat   \c true iff the Pattern must already exist. If this is
 *                     \c false, a new Pattern will be created if needed.
 * \param col          Location for the Column to be set -- must not be
 *                     \c NULL.
 * \param pos          The Event position to be set -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool check_event_reference(Listener* lr,
		const char* types,
		lo_arg** argv,
		Song** song,
		Pattern** pat,
		bool expect_pat,
		Column** col,
		Reltime* pos);


/**
 * Checks that the given Event data is valid.
 *
 * \param lr       The Listener -- must not be \c NULL.
 * \param types    The OSC type description -- must not be \c NULL.
 * \param argv     The OSC arguments -- must not be \c NULL.
 * \param argc     Number of OSC arguments.
 * \param global   \c true iff the Event is located in the global Event
 *                 channel.
 * \param event    The Event to be set -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a event will not be
 *           altered if the function fails.
 */
static bool check_event_data(Listener* lr,
		const char* types,
		lo_arg** argv,
		int argc,
		bool global,
		Event* event);


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
			Event* event = Column_get_edit(col, Reltime_init(RELTIME_AUTO));
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
				event = Column_get_next_edit(col);
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
 * \li \c h   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c s   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */

static bool check_event_reference(Listener* lr,
		const char* types,
		lo_arg** argv,
		Song** song,
		Pattern** pat,
		bool expect_pat,
		Column** col,
		Reltime* pos)
{
	assert(lr != NULL);
	assert(types != NULL);
	assert(argv != NULL);
	assert(song != NULL);
	assert(pat != NULL);
	assert(col != NULL);
	assert(pos != NULL);
	if (types[0] != 'i')
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Song ID is not an integer");
		return false;
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
		return false;
	}
	*song = player->song;
	Pat_table* table = Song_get_pats(*song);
	int32_t pat_num = argv[1]->i;
	if (types[1] != 'i' || pat_num < 0 || pat_num >= 1024) // FIXME: upper limit
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Pattern number");
		return false;
	}
	*pat = Pat_table_get(table, pat_num);
	if (*pat == NULL)
	{
		if (expect_pat)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Couldn't find the Pattern");
			return false;
		}
		*pat = new_Pattern();
		if (*pat == NULL)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Couldn't allocate memory for new Pattern");
			return false;
		}
		if (!Pat_table_set(table, pat_num, *pat))
		{
			del_Pattern(*pat);
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"Couldn't allocate memory for new Pattern");
			return false;
		}
	}
	int32_t col_num = argv[2]->i;
	if (types[2] != 'i' || col_num < 0 || col_num > 64) // FIXME: upper limit
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Column number");
		return false;
	}
	*col = Pattern_global(*pat);
	if (col_num > 0)
	{
		*col = Pattern_col(*pat, col_num - 1);
	}
	int64_t beats = argv[3]->h;
	if (types[3] != 'h')
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid beat number");
		return false;
	}
	int32_t part = argv[4]->i;
	if (types[4] != 'i' || part < 0 || part >= RELTIME_FULL_PART)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event position number");
		return false;
	}
	Reltime_set(pos, beats, part);
	int32_t event_order = argv[5]->i;
	if (types[5] != 'i' || event_order < 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event order index");
		return false;
	}
	return true;
}


static bool check_event_data(Listener* lr,
		const char* types,
		lo_arg** argv,
		int argc,
		bool global,
		Event* event)
{
	assert(lr != NULL);
	assert(types != NULL);
	assert(argv != NULL);
	int32_t event_type = argv[6]->i;
	if (!EVENT_TYPE_IS_VALID(event_type))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event type");
		return false;
	}
	if (!EVENT_TYPE_IS_GENERAL(event_type)
			&& (global != EVENT_TYPE_IS_GLOBAL(event_type)))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Event type is unsupported in this channel");
		return false;
	}
	char* type_desc = Event_type_get_field_types(event_type);
	if (type_desc == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Unused Event type");
		return false;
	}
	int field_count = strlen(type_desc);
	if (argc - 7 < field_count)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Not enough Event type fields");
		return false;
	}
	union { int64_t i; double d; } orig_values[EVENT_FIELDS];
	Event_type orig_type = Event_get_type(event);
	char* orig_types = Event_type_get_field_types(orig_type);
	int orig_count = 0;
	if (orig_types != NULL)
	{
		orig_count = strlen(orig_types);
	}
	for (int i = 0; i < orig_count; ++i)
	{
		bool got_val = false;
		if (orig_types[i] == 'i')
		{
			got_val = Event_int(event, i, &orig_values[i].i);
		}
		else if (orig_types[i] == 'f')
		{
			got_val = Event_float(event, i, &orig_values[i].d);
		}
		assert(got_val);
	}
	bool ok = true;
	Event_reset(event, event_type);
	for (int i = 0; i < field_count; ++i)
	{
		if ((type_desc[i] == 'i' && types[7 + i] != 'h')
				|| (type_desc[i] == 'f' && types[7 + i] != 'd'))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "si",
					"Invalid type of field", (int32_t)i);
			ok = false;
		}
		if (type_desc[i] == 'i')
		{
			if (!Event_set_int(event, i, argv[7 + i]->i))
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "si",
						"Invalid range of field", (int32_t)i);
				ok = false;
			}
		}
		else if (type_desc[i] == 'f')
		{
			if (!Event_set_float(event, i, argv[7 + i]->d))
			{
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "si",
						"Invalid range of field", (int32_t)i);
				ok = false;
			}
		}
	}
	if (!ok)
	{
		Event_reset(event, orig_type);
		for (int i = 0; i < orig_count; ++i)
		{
			bool set_val = false;
			if (orig_types[i] == 'i')
			{
				set_val = Event_set_int(event, i, orig_values[i].i);
			}
			else if (orig_types[i] == 'f')
			{
				set_val = Event_set_float(event, i, orig_values[i].d);
			}
			assert(set_val);
		}
		return false;
	}
#if 0
	if (global)
	{
		switch (event_type)
		{
			case EVENT_TYPE_NOTE_ON:
			case EVENT_TYPE_NOTE_OFF:
				strcpy(lr->method_path + lr->host_path_len, "error");
				lo_send(lr->host, lr->method_path, "s",
						"Event type unsupported by global event channel");
				return false;
			default:
				break;
		}
	}
	else
	{
		switch (event_type)
		{
			case EVENT_TYPE_NOTE_ON:
				event_values[0].i = argv[7]->h;
				if (types[7] != 'h' || event_values[0].i < 0
						|| event_values[0].i >= NOTE_TABLE_NOTES)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid note value for Note On");
					return false;
				}
				event_values[1].i = argv[8]->h;
				if (types[8] != 'h' || event_values[1].i < -1
						|| event_values[1].i >= NOTE_TABLE_NOTE_MODS)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid note modifier for Note On");
					return false;
				}
				event_values[2].i = argv[9]->h;
				if (types[9] != 'h' || event_values[2].i < NOTE_TABLE_OCTAVE_FIRST
						|| event_values[2].i > NOTE_TABLE_OCTAVE_LAST)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid octave for Note On");
					return false;
				}
				event_values[3].i = argv[10]->h;
				if (types[10] != 'h' || event_values[3].i <= 0
						|| event_values[3].i > INSTRUMENTS_MAX)
				{
					strcpy(lr->method_path + lr->host_path_len, "error");
					lo_send(lr->host, lr->method_path, "s",
							"Invalid Instrument number for Note On");
					return false;
				}
				break;
			case EVENT_TYPE_NOTE_OFF:
				break;
			default:
				break;
		}
	}
#endif
	return true;
}


int Listener_pat_del_event(const char* path,
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
	Song* song = NULL;
	Pattern* pat = NULL;
	Column* col = NULL;
	Reltime* pos = Reltime_init(RELTIME_AUTO);
	if (!check_event_reference(lr, types, argv, &song, &pat, true, &col, pos))
	{
		return 0;
	}
	assert(song != NULL);
	assert(pat != NULL);
	assert(col != NULL);
	int32_t player_id = argv[0]->i;
	int32_t pat_num = argv[1]->i;
	int32_t event_order = argv[5]->i;
	Event* event = Column_get_edit(col, pos);
	if (event == NULL || Reltime_cmp(pos, Event_pos(event)) != 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"No Event in the given location");
		return 0;
	}
	for (int32_t i = 0; i < event_order; ++i)
	{
		event = Column_get_next_edit(col);
		if (event == NULL || Reltime_cmp(pos, Event_pos(event)) != 0)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"No Event in the given location");
			return 0;
		}
	}
	if (!Column_remove(col, event))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"No Event in the given location");
		return 0;
	}
	if (!pat_info(lr, player_id, pat_num, pat))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	strcpy(lr->method_path + lr->host_path_len, "events_sent");
	int ret = lo_send(lr->host, lr->method_path, "ii", player_id, pat_num);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_pat_mod_event(const char* path,
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
	Song* song = NULL;
	Pattern* pat = NULL;
	Column* col = NULL;
	Reltime* pos = Reltime_init(RELTIME_AUTO);
	if (!check_event_reference(lr, types, argv, &song, &pat, true, &col, pos))
	{
		return 0;
	}
	assert(song != NULL);
	assert(pat != NULL);
	assert(col != NULL);
	int32_t player_id = argv[0]->i;
	int32_t pat_num = argv[1]->i;
	int32_t col_num = argv[2]->i;
	int32_t event_order = argv[5]->i;
	int32_t event_type = argv[6]->i;
	if (!EVENT_TYPE_IS_VALID(event_type))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event type");
		return 0;
	}
	Event* event = Column_get_edit(col, pos);
	if (event == NULL || Reltime_cmp(pos, Event_pos(event)) != 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"No Event in the given location");
		return 0;
	}
	for (int32_t i = 0; i < event_order; ++i)
	{
		event = Column_get_next_edit(col);
		if (event == NULL || Reltime_cmp(pos, Event_pos(event)) != 0)
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s",
					"No Event in the given location");
			return 0;
		}
	}
	if (!check_event_data(lr, types, argv, argc, col_num == 0, event))
	{
		return 0;
	}
	if (!pat_info(lr, player_id, pat_num, pat))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	strcpy(lr->method_path + lr->host_path_len, "events_sent");
	int ret = lo_send(lr->host, lr->method_path, "ii", player_id, pat_num);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


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
	Song* song = NULL;
	Pattern* pat = NULL;
	Column* col = NULL;
	Reltime* pos = Reltime_init(RELTIME_AUTO);
	if (!check_event_reference(lr, types, argv, &song, &pat, false, &col, pos))
	{
		return 0;
	}
	assert(song != NULL);
	assert(pat != NULL);
	assert(col != NULL);
	int32_t player_id = argv[0]->i;
	int32_t pat_num = argv[1]->i;
	int32_t col_num = argv[2]->i;
	int32_t event_order = argv[5]->i;
	int32_t event_type = argv[6]->i;
	if (!EVENT_TYPE_IS_VALID(event_type))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Invalid Event type");
		return 0;
	}
	Event* event = new_Event(pos, event_type);
	if (event == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s",
				"Couldn't allocate memory for new Event");
		return 0;
	}
	if (!check_event_data(lr, types, argv, argc, col_num == 0, event))
	{
		del_Event(event);
		return 0;
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
	int ret = lo_send(lr->host, lr->method_path, "ii", player_id, pat_num);
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
	assert(ch_num >= 0);
	assert(ch_num <= 64);
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
			Event* event = Column_get_edit(col, Reltime_init(RELTIME_AUTO));
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
				event = Column_get_next_edit(col);
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


