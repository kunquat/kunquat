

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


Playdata* new_Playdata(Ins_table* insts,
                       int buf_count,
                       kqt_frame** bufs)
{
    assert(insts != NULL);
    assert(buf_count > 0);
    assert(bufs != NULL);
    Playdata* play = xalloc(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    play->silent = false;
    play->citer = new_Column_iter(NULL);
    if (play->citer == NULL)
    {
        xfree(play);
        return NULL;
    }
    play->ins_events = new_Event_queue(64);
    if (play->ins_events == NULL)
    {
        del_Column_iter(play->citer);
        xfree(play);
        return NULL;
    }
    play->voice_pool = new_Voice_pool(256, 64);
    if (play->voice_pool == NULL)
    {
        del_Event_queue(play->ins_events);
        del_Column_iter(play->citer);
        xfree(play);
        return NULL;
    }
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        play->channels[i] = new_Channel(insts, i, play->ins_events);
        if (play->channels[i] == NULL)
        {
            for (--i; i >= 0; --i)
            {
                del_Channel(play->channels[i]);
            }
            del_Voice_pool(play->voice_pool);
            del_Event_queue(play->ins_events);
            del_Column_iter(play->citer);
            xfree(play);
            return NULL;
        }
    }
    play->mode = PLAY_SONG;
    play->freq = 48000;
    play->old_freq = play->freq;
    play->subsongs = NULL;
    play->events = NULL;

    play->buf_count = buf_count;
    play->bufs = bufs;
    play->scales = NULL;
    play->active_scale = NULL;

    play->volume = 1;
    play->volume_slide = 0;
    play->volume_slide_target = 1;
    play->volume_slide_frames = 0;
    play->volume_slide_update = 0;

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    play->tempo_slide_target = 0;
    Reltime_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Reltime_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Reltime_init(&play->delay_left);
    play->delay_event_index = -1;

    play->subsong = 0;
    play->section = 0;
    play->pattern = 0;
    Reltime_init(&play->play_time);
    play->play_frames = 0;
    Reltime_init(&play->pos);
    Playdata_reset_stats(play);

    return play;
}


Playdata* new_Playdata_silent(uint32_t freq)
{
    assert(freq > 0);
    Playdata* play = xalloc(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    play->silent = true;
    play->citer = new_Column_iter(NULL);
    if (play->citer == NULL)
    {
        xfree(play);
        return NULL;
    }
    play->ins_events = NULL;
    play->voice_pool = NULL;
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        play->channels[i] = NULL;
    }
    play->mode = PLAY_SONG;
    play->freq = freq;
    play->old_freq = play->freq;
    play->subsongs = NULL;
    play->events = NULL;

    play->volume = 1;
    play->volume_slide = 0;
    play->volume_slide_target = 1;
    play->volume_slide_frames = 0;
    play->volume_slide_update = 0;

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    play->tempo_slide_target = 0;
    Reltime_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Reltime_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Reltime_init(&play->delay_left);
    play->delay_event_index = -1;

    play->subsong = 0;
    play->section = 0;
    play->pattern = 0;
    Reltime_init(&play->play_time);
    play->play_frames = 0;
    Reltime_init(&play->pos);
    Playdata_reset_stats(play);

    play->buf_count = 0;
    play->bufs = NULL;

    return play;
}


void Playdata_set_mix_freq(Playdata* play, uint32_t freq)
{
    assert(play != NULL);
    assert(freq > 0);
    play->freq = freq;
    return;
}


void Playdata_set_subsong(Playdata* play, int subsong)
{
    assert(play != NULL);
    assert(subsong >= 0);
    assert(subsong < KQT_SUBSONGS_MAX);
    play->subsong = subsong;
    play->section = 0;
    if (!play->silent)
    {
        assert(play->voice_pool != NULL);
        Voice_pool_reset(play->voice_pool);
    }
    Subsong* ss = Subsong_table_get(play->subsongs, subsong);
    if (ss == NULL)
    {
        play->tempo = 120;
        return;
    }
    play->tempo = Subsong_get_tempo(ss);
    return;
}


void Playdata_reset_stats(Playdata* play)
{
    assert(play != NULL);
    play->active_voices = 0;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        play->min_amps[i] = INFINITY;
        play->max_amps[i] = -INFINITY;
        play->clipped[i] = 0;
    }
    return;
}


void del_Playdata(Playdata* play)
{
    int i = 0;
    assert(play != NULL);
    if (play->voice_pool != NULL)
    {
        Voice_pool_reset(play->voice_pool);
    }
    for (i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (play->channels[i] != NULL)
        {
            del_Channel(play->channels[i]);
        }
    }
    if (play->citer != NULL)
    {
        del_Column_iter(play->citer);
        play->citer = NULL;
    }
    xfree(play);
    return;
}


