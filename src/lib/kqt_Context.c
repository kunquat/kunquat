

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


#define _POSIX_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <kqt_Context.h>
#include <kqt_Error.h>
#include <kqt_Error_lib.h>

#include <xmemory.h>


kqt_Context* kqt_new_Context(int buf_count,
                             uint32_t buf_size,
                             uint16_t voice_count,
                             uint8_t event_queue_size,
                             kqt_Error* error)
{
    if (buf_count <= 0)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Number of buffers must be positive");
        }
        return NULL;
    }
    if (buf_size <= 0)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Buffer size must be positive");
        }
        return NULL;
    }
    if (voice_count <= 0)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Number of Voices must be positive");
        }
        return NULL;
    }
    if (event_queue_size <= 0)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Event queue size must be positive");
        }
        return NULL;
    }
    if (buf_count > BUF_COUNT_MAX)
    {
        buf_count = BUF_COUNT_MAX;
    }
    if (voice_count > MAX_VOICES)
    {
        voice_count = MAX_VOICES;
    }
    kqt_Context* context = xalloc(kqt_Context);
    if (context == NULL)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Couldn't allocate memory for the Kunquat Context");
        }
        return NULL;
    }
    context->song = NULL;
    context->play = NULL;
    context->voices = NULL;
    context->voices = new_Voice_pool(voice_count, event_queue_size);
    if (context->voices == NULL)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Couldn't allocate memory for Voices"
                                 " in the Kunquat Context");
        }
        kqt_del_Context(context);
        return NULL;
    }
    context->song = new_Song(buf_count, buf_size, event_queue_size);
    if (context->song == NULL)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Couldn't allocate memory for the composition"
                                 " in the Kunquat Context");
        }
        kqt_del_Context(context);
        return NULL;
    }
    context->play = new_Playdata(44100, context->voices, Song_get_insts(context->song));
    if (context->play == NULL)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Couldn't allocate memory for the playback information"
                                 " in the Kunquat Context");
        }
        kqt_del_Context(context);
        return NULL;
    }
    context->play->order = Song_get_order(context->song);
    context->play->events = Song_get_events(context->song);
    return context;
}


kqt_Context* kqt_new_Context_from_path(char* path,
                                       uint32_t buf_size,
                                       uint16_t voice_count,
                                       uint8_t event_queue_size,
                                       kqt_Error* error)
{
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "Couldn't access %s: %s", path, strerror(errno));
        }
        return NULL;
    }
    kqt_Context* context = kqt_new_Context(1, buf_size, voice_count, event_queue_size, error);
    if (context == NULL)
    {
        return NULL;
    }
    File_tree* tree = NULL;
    Read_state* state = READ_STATE_AUTO;
    if (S_ISDIR(info->st_mode))
    {
        tree = new_File_tree_from_fs(path, state);
        if (tree == NULL)
        {
            if (error != NULL)
            {
                kqt_Error_set(error, "%s:%d: %s", state->path, state->row, state->message);
            }
            kqt_del_Context(context);
            return NULL;
        }
    }
    else
    {
        tree = new_File_tree_from_tar(path, state);
        if (tree == NULL)
        {
            if (error != NULL)
            {
                kqt_Error_set(error, "%s:%d: %s", state->path, state->row, state->message);
            }
            kqt_del_Context(context);
            return NULL;
        }
    }
    assert(tree != NULL);
    if (!Song_read(context->song, tree, state))
    {
        if (error != NULL)
        {
            kqt_Error_set(error, "%s:%d: %s", state->path, state->row, state->message);
        }
        kqt_del_Context(context);
        return NULL;
    }
    return context;
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


void kqt_del_Context(kqt_Context* context)
{
    if (context == NULL)
    {
        return;
    }
    if (context->play != NULL)
    {
        del_Playdata(context->play);
    }
    if (context->voices != NULL)
    {
        del_Voice_pool(context->voices);
    }
    if (context->song != NULL)
    {
        del_Song(context->song);
    }
    xfree(context);
    return;
}


