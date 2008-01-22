

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

#include <Real.h>

#include "Song.h"

#include <xmemory.h>


Song* new_Song(int buf_count, uint32_t buf_size)
{
	assert(buf_count >= 1);
	assert(buf_count <= BUF_COUNT_MAX);
	assert(buf_size > 0);
	Song* song = xalloc(Song);
	if (song == NULL)
	{
		return NULL;
	}
	song->buf_count = buf_count;
//	song->buf = NULL;
	song->order = NULL;
	song->pats = NULL;
	song->insts = NULL;
	song->notes = NULL;
/*	song->buf = xalloc(frame_t*);
	if (song->buf == NULL)
	{
		del_Song(song);
		return NULL;
	} */
	for (int i = 0; i < buf_count; ++i)
	{
		song->buf[i] = xnalloc(frame_t, buf_size);
		if (song->buf[i] == NULL)
		{
			del_Song(song);
			return NULL;
		}
		for (uint32_t k = 0; k < buf_size; ++k)
		{
			song->buf[i][k] = 0;
		}
	}
	song->order = new_Order();
	if (song->order == NULL)
	{
		del_Song(song);
		return NULL;
	}
	song->pats = new_Pat_table(1024);
	if (song->pats == NULL)
	{
		del_Song(song);
		return NULL;
	}
	song->insts = new_Ins_table(256);
	if (song->insts == NULL)
	{
		del_Song(song);
		return NULL;
	}
	song->notes = new_Note_table(L"12-TET",
			523.25113060119725,
			Real_init_as_frac(REAL_AUTO, 2, 1));
	if (song->notes == NULL)
	{
		del_Song(song);
		return NULL;
	}
	wchar_t* note_names[12] =
			{ L"C",  L"C#", L"D",  L"D#", L"E",  L"F",
			  L"F#", L"G",  L"G#", L"A",  L"A#", L"B" };
	Note_table_set_note(song->notes,
			0,
			note_names[0],
			Real_init_as_frac(REAL_AUTO, 1, 1));
	for (int i = 1; i < 12; ++i)
	{
		Note_table_set_note_cents(song->notes,
				i,
				note_names[i],
				i * 100);
	}
	song->name[0] = song->name[SONG_NAME_MAX - 1] = L'\0';
	song->tempo = 120;
	song->mix_vol = 0;
	song->global_vol = 0;
	return song;
}


uint32_t Song_mix(Song* song, uint32_t nframes, Playdata* play)
{
	assert(song != NULL);
	assert(play != NULL);
	if (play->play == STOP)
	{
		return 0;
	}
	uint32_t mixed = 0;
	while (mixed < nframes && play->play)
	{
		Pattern* pat = NULL;
		if (play->pattern >= 0)
		{
			pat = Pat_table_get(song->pats, play->pattern);
		}
		if (pat == NULL)
		{
			// TODO: Stop or restart?
			play->play = STOP;
			break;
		}
		uint32_t proc_start = mixed;
		mixed += Pattern_mix(pat, nframes, mixed, play);
		Event* event = NULL;
		uint32_t proc_until = mixed;
		Event_queue_get(song->events, &event, &proc_until);
		// TODO: mix private instrument buffers
		while (proc_start < mixed)
		{
			assert(proc_until <= mixed);
			for (uint32_t i = proc_start; i < proc_until; ++i)
			{
				// TODO: modify buffer according to state
			}
			proc_start = proc_until;
			proc_until = mixed;
			while (Event_queue_get(song->events, &event, &proc_until))
			{
				// TODO: process events
				if (proc_start < mixed)
				{
					break;
				}
			}
		}
		assert(!Event_queue_get(song->events, &event, &proc_until));
	}
	return mixed;
}


void del_Song(Song* song)
{
	assert(song != NULL);
	if (song->buf != NULL)
	{
		for (int i = 0; i < song->buf_count && song->buf[i] != NULL; ++i)
		{
			xfree(song->buf[i]);
		}
//		xfree(song->buf);
	}
	if (song->order != NULL)
	{
		del_Order(song->order);
	}
	if (song->pats != NULL)
	{
		del_Pat_table(song->pats);
	}
	if (song->insts != NULL)
	{
		del_Ins_table(song->insts);
	}
	if (song->notes != NULL)
	{
		del_Note_table(song->notes);
	}
	xfree(song);
	return;
}


