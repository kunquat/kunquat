

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

#include "Instrument.h"
#include "Instrument_debug.h"

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
	ins->pbufs = NULL;
	ins->bufs = ins->gbufs = bufs;
	ins->buf_len = buf_len;
	switch (type)
	{
		case INS_TYPE_DEBUG:
			ins->mix = Instrument_debug_mix;
			break;
		default:
			ins->mix = NULL;
	}
	return ins;
}


void Instrument_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(state != NULL);
	assert(nframes <= ins->buf_len);
	assert(freq > 0);
	assert(ins->mix != NULL);
	if (!state->active)
	{
		return;
	}
	ins->mix(ins, state, nframes, offset, freq);
	return;
}


void del_Instrument(Instrument* ins)
{
	assert(ins != NULL);
	xfree(ins->events);
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


