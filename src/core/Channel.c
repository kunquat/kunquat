

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

#include "Channel.h"

#include <Reltime.h>
#include <Event.h>
#include <Column.h>

#include <xmemory.h>


Channel* new_Channel(Ins_table* insts)
{
	assert(insts != NULL);
	Channel* ch = xalloc(Channel);
	if (ch == NULL)
	{
		return NULL;
	}
	ch->note_off = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_OFF);
	if (ch->note_off == NULL)
	{
		xfree(ch);
		return NULL;
	}
	ch->insts = insts;
	ch->fg = NULL;
	ch->fg_id = 0;
	return ch;
}


void Channel_set_voices(Channel* ch,
		Voice_pool* pool,
		Column* col,
		Reltime* start,
		Reltime* end,
		uint32_t offset,
		double tempo,
		uint32_t freq)
{
	assert(ch != NULL);
	assert(pool != NULL);
	assert(col != NULL);
	assert(start != NULL);
	assert(end != NULL);
	assert(tempo > 0);
	assert(freq > 0);
	Event* next = Column_get(col, start);
	if (next == NULL)
	{
		return;
	}
	Reltime* next_pos = Event_pos(next);
	while (Reltime_cmp(next_pos, end) < 0)
	{
		assert(Reltime_cmp(start, next_pos) <= 0);
		if (Event_get_type(next) == EVENT_TYPE_NOTE_ON)
		{
			if (ch->fg != NULL)
			{
				// move the old Voice to the background
				ch->fg = Voice_pool_get_voice(pool, ch->fg,	ch->fg_id);
				if (ch->fg == NULL)
				{
					// The Voice has been given to another channel -- giving up
					next = Column_get_next(col);
					if (next == NULL)
					{
						break;
					}
					next_pos = Event_pos(next);
					continue;
				}
				Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
				uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq)
						+ offset;
				if (!Voice_add_event(ch->fg, ch->note_off, abs_pos))
				{
					// Kill the Voice so that it doesn't
					// stay active indefinitely
					Voice_reset(ch->fg);
					ch->fg = NULL;
					ch->fg_id = 0;
					// TODO: notify in case of failure
				}
			}
			int64_t num = 0;
			Event_int(next, 3, &num);
			if (num <= 0)
			{
				next = Column_get_next(col);
				if (next == NULL)
				{
					break;
				}
				next_pos = Event_pos(next);
				continue;
			}
			Instrument* ins = Ins_table_get(ch->insts, (int)num);
			if (ins == NULL)
			{
				next = Column_get_next(col);
				if (next == NULL)
				{
					break;
				}
				next_pos = Event_pos(next);
				continue;
			}
			// allocate new voice
			ch->fg = Voice_pool_get_voice(pool, NULL, 0);
			assert(ch->fg != NULL);
			ch->fg_id = Voice_id(ch->fg);
			Voice_init(ch->fg, ins);
			Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
			uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq)
					+ offset;
			if (!Voice_add_event(ch->fg, next, abs_pos))
			{
				// This really shouldn't occur here!
				//  - implies that Voice is uninitialised
				//    or cannot contain any events
				assert(false);
			}
		}
		else if (ch->fg != NULL &&
				!EVENT_TYPE_IS_GLOBAL(Event_get_type(next)))
		{
			ch->fg = Voice_pool_get_voice(pool,	ch->fg, ch->fg_id);
			if (ch->fg == NULL)
			{
				// The Voice has been given to another channel -- giving up
				next = Column_get_next(col);
				if (next == NULL)
				{
					break;
				}
				next_pos = Event_pos(next);
				continue;
			}
			Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
			uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq)
					+ offset;
			if (!Voice_add_event(ch->fg, next, abs_pos))
			{
				Voice_reset(ch->fg);
				ch->fg = NULL;
				ch->fg_id = 0;
				// TODO: notify in case of failure
			}
		}
		next = Column_get_next(col);
		if (next == NULL)
		{
			break;
		}
		next_pos = Event_pos(next);
	}
	return;
}


void Channel_reset(Channel* ch)
{
	assert(ch != NULL);
	ch->fg = NULL;
	ch->fg_id = 0;
	return;
}


void del_Channel(Channel* ch)
{
	assert(ch != NULL);
	xfree(ch->note_off);
	xfree(ch);
	return;
}


