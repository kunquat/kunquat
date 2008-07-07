

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

#include <AAtree.h>
#include <Instrument.h>
#include "Instrument_pcm.h"
#include <Sample.h>

#include <xmemory.h>


typedef struct freq_entry
{
	double min_freq;
	double sample_freq;
	uint16_t index;
} freq_entry;


static int freq_entry_cmp(freq_entry* f1, freq_entry* f2);


typedef struct pcm_type_data
{
	AAtree* freq_map;
	Sample* samples[PCM_SAMPLES_MAX];
} pcm_type_data;


int Instrument_pcm_init(Instrument* ins)
{
	assert(ins != NULL);
	pcm_type_data* type_data = xalloc(pcm_type_data);
	if (type_data == NULL)
	{
		return 1;
	}
	type_data->freq_map = new_AAtree(
			(int (*)(void*, void*))freq_entry_cmp, free);
	if (type_data->freq_map == NULL)
	{
		xfree(type_data);
		return 1;
	}
	for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
	{
		type_data->samples[i] = NULL;
	}
	ins->type_data = type_data;
	return 0;
}


void Instrument_pcm_uninit(Instrument* ins)
{
	assert(ins != NULL);
	assert(ins->type_data != NULL);
	pcm_type_data* type_data = ins->type_data;
	for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
	{
		if (type_data->samples[i] != NULL)
		{
			del_Sample(type_data->samples[i]);
		}
	}
	del_AAtree(type_data->freq_map);
	xfree(ins->type_data);
	return;
}


bool Instrument_pcm_set_sample(Instrument* ins,
		uint16_t index,
		char* path)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(index < PCM_SAMPLES_MAX);
	pcm_type_data* type_data = ins->type_data;
	if (path == NULL)
	{
		if (type_data->samples[index] != NULL)
		{
			del_Sample(type_data->samples[index]);
			type_data->samples[index] = NULL;
		}
		return true;
	}
	Sample* sample = new_Sample();
	if (sample == NULL)
	{
		return false;
	}
	if (!Sample_load_path(sample, path, SAMPLE_FORMAT_WAVPACK))
	{
		del_Sample(sample);
		return false;
	}
	if (type_data->samples[index] != NULL)
	{
		del_Sample(type_data->samples[index]);
		type_data->samples[index] = NULL;
	}
	type_data->samples[index] = sample;
	return true;
}


Sample* Instrument_pcm_get_sample(Instrument* ins, uint16_t index)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(index < PCM_SAMPLES_MAX);
	pcm_type_data* type_data = ins->type_data;
	return type_data->samples[index];
}


char* Instrument_pcm_get_path(Instrument* ins, uint16_t index)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(index < PCM_SAMPLES_MAX);
	pcm_type_data* type_data = ins->type_data;
	if (type_data->samples[index] == NULL)
	{
		return NULL;
	}
	return Sample_get_path(type_data->samples[index]);
}


void Instrument_pcm_set_sample_freq(Instrument* ins,
		uint16_t index,
		double freq)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(index < PCM_SAMPLES_MAX);
	assert(freq > 0);
	pcm_type_data* type_data = ins->type_data;
	if (type_data->samples[index] == NULL)
	{
		return;
	}
	Sample_set_freq(type_data->samples[index], freq);
	return;
}


double Instrument_pcm_get_sample_freq(Instrument* ins, uint16_t index)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(index < PCM_SAMPLES_MAX);
	pcm_type_data* type_data = ins->type_data;
	if (type_data->samples[index] == NULL)
	{
		return 0;
	}
	return Sample_get_freq(type_data->samples[index]);
}


static Sample* state_to_sample(Instrument* ins, Voice_state* state);


void Instrument_pcm_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
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
	Sample* sample = state_to_sample(ins, state);
	if (sample == NULL)
	{
		state->active = false;
		return;
	}
	Sample_mix(sample, ins->bufs, state, nframes, offset, freq);
	return;
}


static Sample* state_to_sample(Instrument* ins, Voice_state* state)
{
	assert(ins != NULL);
	assert(ins->type == INS_TYPE_PCM);
	assert(ins->type_data != NULL);
	assert(state != NULL);
	pcm_type_data* type_data = ins->type_data;
	return type_data->samples[0];
}


static int freq_entry_cmp(freq_entry* f1, freq_entry* f2)
{
	assert(f1 != NULL);
	assert(f2 != NULL);
	if (f1->min_freq < f2->min_freq)
	{
		return 1;
	}
	else if (f1->min_freq > f2->min_freq)
	{
		return -1;
	}
	return 0;
}


