

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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
#include <math.h>

#include <Voice_pool.h>
#include <Channel.h>
#include <Reltime.h>

#include <Playdata.h>

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
    for (int i = 0; i < COLUMNS_MAX; ++i)
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
    Playdata_reset_stats(play);
    return play;
}


void Playdata_set_mix_freq(Playdata* play, uint32_t freq)
{
    assert(play != NULL);
    assert(freq > 0);
    play->freq = freq;
    return;
}


void Playdata_reset_stats(Playdata* play)
{
    assert(play != NULL);
    play->active_voices = 0;
    return;
}


void del_Playdata(Playdata* play)
{
    int i = 0;
    assert(play != NULL);
    Voice_pool_reset(play->voice_pool);
    for (i = 0; i < COLUMNS_MAX; ++i)
    {
        assert(play->channels[i] != NULL);
        del_Channel(play->channels[i]);
    }
    xfree(play);
    return;
}


