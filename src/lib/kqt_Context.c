

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
#include <math.h>

#include <kqt_Context.h>

#include <xmemory.h>


kqt_Context* new_kqt_Context(uint32_t freq, uint16_t voices, Song* song)
{
    static int32_t id = 0;
    assert(freq > 0);
    assert(voices > 0);
    assert(voices < MAX_VOICES);
    assert(song != NULL);
    kqt_Context* context = xalloc(kqt_Context);
    if (context == NULL)
    {
        return NULL;
    }
    context->voices = new_Voice_pool(voices, 32); // TODO: event count
    if (context->voices == NULL)
    {
        xfree(context);
        return NULL;
    }
    context->song = song;
    context->play = new_Playdata(freq, context->voices, Song_get_insts(song));
    if (context->play == NULL)
    {
        del_Voice_pool(context->voices);
        xfree(context);
        return NULL;
    }
    context->play->order = Song_get_order(song);
    context->play->events = Song_get_events(song);
    context->id = id++;
    return context;
}


int32_t kqt_Context_get_id(kqt_Context* context)
{
    assert(context != NULL);
    return context->id;
}


Song* kqt_Context_get_song(kqt_Context* context)
{
    assert(context != NULL);
    return context->song;
}


Playdata* kqt_Context_get_playdata(kqt_Context* context)
{
    assert(context != NULL);
    return context->play;
}


uint32_t kqt_Context_mix(kqt_Context* context, uint32_t nframes)
{
    assert(context != NULL);
    if (!context->play || context->song == NULL)
    {
        return 0;
    }
    return Song_mix(context->song, nframes, context->play);
}


void kqt_Context_play_pattern(kqt_Context* context, int16_t num, double tempo)
{
    assert(context != NULL);
    assert(num >= 0);
    assert(num < PATTERNS_MAX);
    assert(isfinite(tempo));
    assert(tempo > 0);
    kqt_Context_stop(context);
    context->play->pattern = num;
    context->play->tempo = tempo;
    Playdata_reset_stats(context->play);
    context->play->mode = PLAY_PATTERN;
    return;
}


void kqt_Context_play_subsong(kqt_Context* context, uint16_t subsong)
{
    assert(context != NULL);
    assert(subsong < SUBSONGS_MAX);
    kqt_Context_stop(context);
    context->play->subsong = subsong;
    Subsong* ss = Order_get_subsong(context->play->order, context->play->subsong);
    if (ss == NULL)
    {
        context->play->tempo = 120;
    }
    else
    {
        context->play->tempo = Subsong_get_tempo(ss);
    }
    Playdata_reset_stats(context->play);
    context->play->mode = PLAY_SONG;
    return;
}


void kqt_Context_play_song(kqt_Context* context)
{
    assert(context != NULL);
    kqt_Context_stop(context);
    context->play->subsong = Song_get_subsong(context->song);
    Playdata_reset_stats(context->play);
    context->play->mode = PLAY_SONG;
    return;
}


void kqt_Context_play_event(kqt_Context* context)
{
    assert(context != NULL);
    if (context->play->mode >= PLAY_EVENT)
    {
        return;
    }
    kqt_Context_stop(context);
    Playdata_reset_stats(context->play);
    context->play->mode = PLAY_EVENT;
    return;
}


void kqt_Context_stop(kqt_Context* context)
{
    assert(context != NULL);
    context->play->mode = STOP;
    Voice_pool_reset(context->voices);
    for (int i = 0; i < COLUMNS_MAX; ++i)
    {
        Channel_reset(context->play->channels[i]);
    }
    kqt_Reltime_init(&context->play->play_time);
    context->play->play_frames = 0;
    context->play->subsong = Song_get_subsong(context->song);
    Subsong* ss = Order_get_subsong(context->play->order, context->play->subsong);
    if (ss == NULL)
    {
        context->play->tempo = 120;
    }
    else
    {
        context->play->tempo = Subsong_get_tempo(ss);
    }
    context->play->order_index = 0;
    context->play->pattern = 0;
    kqt_Reltime_init(&context->play->pos);
    return;
}


void kqt_Context_set_mix_freq(kqt_Context* context, uint32_t freq)
{
    assert(context != NULL);
    assert(freq > 0);
    Playdata_set_mix_freq(context->play, freq);
    return;
}


void del_kqt_Context(kqt_Context* context)
{
    assert(context != NULL);
    del_Playdata(context->play);
    del_Voice_pool(context->voices);
    if (context->song != NULL)
    {
        del_Song(context->song);
    }
    xfree(context);
    return;
}


