

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
#include <math.h>

#include "Instrument_sine.h"


#define PI (3.14159265358979323846)


void Instrument_sine_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(state != NULL);
	assert(nframes <= ins->buf_len);
	assert(freq > 0);
	if (!state->active)
	{
		return;
	}
	for (uint32_t i = offset; i < nframes; ++i)
	{
		double val_l = 0;
		double val_r = 0;
		val_l = val_r = sin(state->rel_pos_part) / 3;
		if (!state->note_on)
		{
			val_l = val_r = (val_l / 2) * (1 - state->noff_pos_part);
		}
		state->rel_pos_part += state->freq * PI / freq;
		ins->bufs[0][i] += val_l;
		ins->bufs[1][i] += val_r;
		if (!state->note_on)
		{
			state->noff_pos_part += 1.0 / freq;
			if (state->noff_pos_part >= 1)
			{
				state->active = false;
				return;
			}
		}
	}
	return;
}


