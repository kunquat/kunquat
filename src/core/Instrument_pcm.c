

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
#include <math.h>

#include "Instrument_pcm.h"
#include <Sample.h>

#include <xmemory.h>


typedef struct pcm_type_data
{
	Sample* sample;
} pcm_type_data;


int Instrument_pcm_init(Instrument* ins)
{
	assert(ins != NULL);
	pcm_type_data* type_data = xalloc(pcm_type_data);
	if (type_data == NULL)
	{
		return 1;
	}
	type_data->sample = new_Sample();
	if (type_data->sample == NULL)
	{
		xfree(type_data);
		return 1;
	}
	ins->type_data = type_data;
	return 0;
}


void Instrument_pcm_uninit(Instrument* ins)
{
	assert(ins != NULL);
	assert(ins->type_data != NULL);
	pcm_type_data* type_data = ins->type_data;
	if (type_data->sample != NULL)
	{
		del_Sample(type_data->sample);
	}
	xfree(ins->type_data);
	return;
}


void Instrument_pcm_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(ins->type_data != NULL);
	assert(state != NULL);
//	assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
	assert(freq > 0);
	assert(ins->bufs[0] != NULL);
	assert(ins->bufs[1] != NULL);
	if (!state->active)
	{
		return;
	}
	pcm_type_data* type_data = ins->type_data;
	if (type_data->sample == NULL)
	{
		state->active = false;
		return;
	}
	Sample_mix(type_data->sample, ins->bufs, state, nframes, offset, freq);
	return;
}


