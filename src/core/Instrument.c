

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

#include "Instrument.h"
#include "Instrument_debug.h"
#include "Instrument_sine.h"
#include "Instrument_pcm.h"

#include <xmemory.h>


Instrument* new_Instrument(Ins_type type,
		frame_t** bufs,
		uint32_t buf_len,
		uint8_t events)
{
	assert(type > INS_TYPE_NONE);
	assert(type < INS_TYPE_LAST);
	assert(bufs != NULL);
	assert(bufs[0] != NULL);
	assert(bufs[1] != NULL);
	assert(buf_len > 0);
	assert(events > 0);
	Instrument* ins = xalloc(Instrument);
	if (ins == NULL)
	{
		return NULL;
	}
	ins->events = new_Event_queue(events);
	if (ins->events == NULL)
	{
		xfree(ins);
		return NULL;
	}
	ins->type = type;
	ins->name[0] = ins->name[INS_NAME_MAX - 1] = L'\0';
	ins->pbufs = NULL;
	ins->bufs = ins->gbufs = bufs;
	ins->buf_len = buf_len;
	ins->type_data = NULL;
	ins->type_desc = NULL;
	ins->get_field = NULL;
	ins->set_field = NULL;
	switch (type)
	{
		case INS_TYPE_DEBUG:
			ins->mix = Instrument_debug_mix;
			ins->init = NULL;
			ins->uninit = NULL;
			break;
		case INS_TYPE_SINE:
			ins->mix = Instrument_sine_mix;
			ins->init = NULL;
			ins->uninit = NULL;
			break;
		case INS_TYPE_PCM:
			ins->mix = Instrument_pcm_mix;
			ins->init = Instrument_pcm_init;
			ins->uninit = Instrument_pcm_uninit;
			ins->get_field = Instrument_pcm_get_field;
			ins->set_field = Instrument_pcm_set_field;
			break;
		default:
			ins->init = NULL;
			ins->uninit = NULL;
			ins->mix = NULL;
			assert(false);
	}
	assert((ins->init == NULL) == (ins->uninit == NULL));
	if (ins->init != NULL)
	{
		if (ins->init(ins) != 0)
		{
			del_Event_queue(ins->events);
			xfree(ins);
			return NULL;
		}
	}
	ins->notes = NULL;
	return ins;
}


Ins_type Instrument_get_type(Instrument* ins)
{
	assert(ins != NULL);
	return ins->type;
}


Instrument_field* Instrument_get_type_desc(Instrument* ins)
{
	assert(ins != NULL);
	return ins->type_desc;
}


bool Instrument_get_field(Instrument* ins, int index, void* data, char** type)
{
	assert(ins != NULL);
	assert(index >= 0);
	assert(data != NULL);
	if (ins->get_field == NULL)
	{
		return false;
	}
	return ins->get_field(ins, index, data, type);
}


bool Instrument_set_field(Instrument* ins, int index, void* data)
{
	assert(ins != NULL);
	assert(index >= 0);
	assert(data != NULL);
	if (ins->set_field == NULL)
	{
		return false;
	}
	return ins->set_field(ins, index, data);
}


void Instrument_set_name(Instrument* ins, wchar_t* name)
{
	assert(ins != NULL);
	assert(name != NULL);
	wcsncpy(ins->name, name, INS_NAME_MAX - 1);
	ins->name[INS_NAME_MAX - 1] = L'\0';
	return;
}


wchar_t* Instrument_get_name(Instrument* ins)
{
	assert(ins != NULL);
	return ins->name;
}


void Instrument_set_note_table(Instrument* ins, Note_table** notes)
{
	assert(ins != NULL);
	assert(notes != NULL);
	ins->notes = notes;
	return;
}


void Instrument_process_note(Instrument* ins,
		Voice_state* state,
		int note,
		int mod,
		int octave)
{
	assert(ins != NULL);
	assert(state != NULL);
	assert(note >= 0);
	assert(note < NOTE_TABLE_NOTES);
	assert(mod < NOTE_TABLE_NOTE_MODS);
	assert(octave >= NOTE_TABLE_OCTAVE_FIRST);
	assert(octave <= NOTE_TABLE_OCTAVE_LAST);
	if (ins->notes == NULL || *ins->notes == NULL)
	{
		return;
	}
	pitch_t freq = Note_table_get_pitch(*ins->notes, note, mod, octave);
	if (freq > 0)
	{
		state->freq = freq;
	}
	return;
}


void Instrument_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(state != NULL);
//	assert(nframes <= ins->buf_len);
	assert(freq > 0);
	assert(ins->mix != NULL);
	if (!state->active)
	{
		return;
	}
	if (ins->mix != NULL)
	{
		ins->mix(ins, state, nframes, offset, freq);
	}
	return;
}


void del_Instrument(Instrument* ins)
{
	assert(ins != NULL);
	del_Event_queue(ins->events);
	if (ins->uninit != NULL)
	{
		ins->uninit(ins);
	}
	if (ins->pbufs != NULL)
	{
		assert(ins->pbufs[0] != NULL);
		assert(ins->pbufs[1] != NULL);
		xfree(ins->pbufs[0]);
		xfree(ins->pbufs[1]);
		xfree(ins->pbufs);
	}
	xfree(ins);
}


