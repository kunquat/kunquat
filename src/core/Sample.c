

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
#include <stdbool.h>
#include <math.h>

#include "Sample.h"

#include <xmemory.h>


Sample* new_Sample(void)
{
	Sample* sample = xalloc(Sample);
	if (sample == NULL)
	{
		return NULL;
	}
	sample->path = NULL;
	sample->format = SAMPLE_FORMAT_NONE;
	sample->changed = NULL;
	sample->is_lossy = false;
	sample->channels = 1;
	sample->bits = 16;
	sample->is_float = false;
	sample->len = 0;
	sample->mid_freq = 48000;
	sample->data[0] = NULL;
	sample->data[1] = NULL;
	// XXX: For testing -- remove later
	double* buf_l = xnalloc(double, 32*50);
	if (buf_l == NULL)
	{
		xfree(sample);
		return NULL;
	}
	double vol = 1;
	for (int i = 0; i < 50; ++i)
	{
		for (int k = 0; k < 32; ++k)
		{
			buf_l[(i*32)+k] = (k - 16) * vol / 48;
			vol -= 1.0 / (32*50);
		}
	}
	sample->is_float = true;
	sample->data[0] = buf_l;
	sample->bits = 32;
	sample->len = 32*50;
	sample->mid_freq = 32*440;
	// XXX: Testing ends
	return sample;
}


bool Sample_load(Sample* sample, FILE* in, Sample_format format)
{
	assert(sample != NULL);
	assert(in != NULL);
	assert(format > SAMPLE_FORMAT_NONE);
	assert(format < SAMPLE_FORMAT_LAST);
	return false; // TODO: implement
}


void Sample_mix(Sample* sample,
		frame_t** bufs,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(sample != NULL);
	assert(bufs != NULL);
	assert(bufs[0] != NULL);
	assert(bufs[1] != NULL);
	assert(state != NULL);
	assert(freq > 0);
	if (!state->active)
	{
		return;
	}
	for (uint32_t i = offset; i < nframes; ++i)
	{
		if (state->pos >= sample->len)
		{
			state->active = false;
			break;
		}
		if (sample->is_float)
		{
			double* buf_l = sample->data[0];
//			double* buf_r = sample->data[1];
			double cur = buf_l[state->pos];
			double next = 0;
			if (state->pos + 1 < sample->len)
			{
				next = buf_l[state->pos + 1];
			}
			double val_l = 0;
			double val_r = 0;
			val_l = val_r = cur * (1 - state->pos_rem)
					+ next * (state->pos_rem);
			bufs[0][i] += val_l;
			bufs[1][i] += val_r;
		}
		double advance = (state->freq / 440) * sample->mid_freq / freq;
		uint64_t adv = floor(advance);
		double adv_rem = advance - adv;
		state->pos += adv;
		state->pos_rem += adv_rem;
		if (state->pos_rem >= 1)
		{
			state->pos_rem -= 1;
			++state->pos;
		}
	}
	return;
}


void del_Sample(Sample* sample)
{
	assert(sample != NULL);
	if (sample->path != NULL)
	{
		xfree(sample->path);
	}
	if (sample->data[0] != NULL)
	{
		xfree(sample->data[0]);
	}
	if (sample->data[1] != NULL)
	{
		xfree(sample->data[1]);
	}
	xfree(sample);
	return;
}


