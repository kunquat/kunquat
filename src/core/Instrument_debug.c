

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#include "Instrument_debug.h"


void Instrument_debug_mix(Instrument* ins,
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
		if (state->rel_pos == 0)
		{
			val_l = 1.0;
			val_r = 1.0;
			state->rel_pos = 1;
		}
		else
		{
			val_l = 0.5;
			val_r = 0.5;
		}
		if (!state->note_on)
		{
			val_l = -val_l;
			val_r = -val_r;
		}
		ins->bufs[0][i] += val_l;
		ins->bufs[1][i] += val_r;
		state->rel_pos_part += state->freq / freq;
		if (!state->note_on)
		{
			state->noff_pos_part += state->freq / freq;
			if (state->noff_pos_part >= 2)
			{
				state->active = false;
				return;
			}
		}
		if (state->rel_pos_part >= 1)
		{
			++state->pos;
			if (state->pos >= 10)
			{
				state->active = false;
				return;
			}
			state->rel_pos = 0;
			state->rel_pos_part -= floor(state->rel_pos_part);
		}
	}
	return;
}


