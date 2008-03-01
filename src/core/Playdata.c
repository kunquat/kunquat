

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
#include <string.h>

#include <Voice_pool.h>
#include <Channel.h>
#include <Reltime.h>

#include "Playdata.h"

#include <xmemory.h>


Playdata* new_Playdata(uint32_t freq, Voice_pool* pool, Ins_table* insts)
{
	assert(freq > 0);
	assert(pool != NULL);
	Playdata* play = xalloc(Playdata);
	if (play == NULL)
	{
		return NULL;
	}
	play->voice_pool = pool;
	for (int i = 0; i < PAT_CHANNELS; ++i)
	{
		play->channels[i] = new_Channel(insts);
		if (play->channels[i] == NULL)
		{
			for (--i; i >= 0; --i)
			{
				del_Channel(play->channels[i]);
			}
			xfree(play);
			return NULL;
		}
	}
	play->mode = STOP;
	play->freq = freq;
	play->order = NULL;
	play->events = NULL;
	play->tempo = 0;
	play->subsong = 0;
	play->order_index = 0;
	play->pattern = 0;
	Reltime_init(&play->play_time);
	play->play_frames = 0;
	Reltime_init(&play->pos);
	return play;
}


#if 0
int Playdata_process_jack(jack_nframes_t nframes, void* arg)
{
	Playdata* data = NULL;
	jack_default_audio_sample_t* out_l = NULL;
	jack_default_audio_sample_t* out_r = NULL;
	assert(arg != NULL);
	data = (Playdata*)arg;
	if (!data->play)
	{
		return 0;
	}
	assert(data->active);
	assert(data->play > STOP);
	assert(data->play < PLAY_LAST);
	assert(data->freq > 0);
	assert(data->song != NULL);
	out_l = jack_port_get_buffer(data->ports[0], nframes);
	out_r = jack_port_get_buffer(data->ports[1], nframes);
	memset(out_l, 0, nframes * sizeof(jack_default_audio_sample_t));
	memset(out_r, 0, nframes * sizeof(jack_default_audio_sample_t));
	if (data->tempo <= 0)
	{
		assert(data->song->tempo > 0);
		data->tempo = data->song->tempo;
		data->order = 0;
		Duration_init(&(data->pos));
	}
	if (data->play == PLAY_PATTERN)
	{
		Duration addition;
		unsigned long mixed = 0;
		Pattern* cur = NULL;
		memset(out_l, 0, sizeof(jack_default_audio_sample_t) * nframes);
		cur = Song_get_pattern(data->song, data->pattern);
		if (cur == NULL)
		{
			data->play = 0;
			return 0;
		}
		mixed = Pattern_mix(cur, data->channels, &(data->pos), data->song->instruments, nframes, out_l, data->freq);
		Duration_add(Duration_fromframes(&addition, mixed, 120, data->freq), &(data->pos), &(data->pos));
		memcpy(out_r, out_l, sizeof(jack_default_audio_sample_t) * nframes);
		if (mixed < nframes)
		{
			memset(out_l + mixed, 0, sizeof(jack_default_audio_sample_t) * (nframes - mixed));
			memset(out_r + mixed, 0, sizeof(jack_default_audio_sample_t) * (nframes - mixed));
			data->play = 0;
			Duration_set(&(data->pos), 0, 0);
		}
	}
	return 0;
}
#endif


void del_Playdata(Playdata* play)
{
	int i = 0;
	assert(play != NULL);
	Voice_pool_reset(play->voice_pool);
	for (i = 0; i < PAT_CHANNELS; ++i)
	{
		assert(play->channels[i] != NULL);
		del_Channel(play->channels[i]);
	}
	xfree(play);
	return;
}


