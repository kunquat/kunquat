

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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <kqt_Context.h>

#include <Song_limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>
#include <File_base.h> // for string parsing

#include <xmemory.h>


#define KQT_CONTEXT_ERROR_LENGTH (256)


struct kqt_Context
{
    Song* song;
    Playdata* play;
    Playdata* play_silent;
    Voice_pool* voices;
    char error[KQT_CONTEXT_ERROR_LENGTH];
};


void kqt_Context_set_error(kqt_Context* context, char* message, ...);

void kqt_Context_stop(kqt_Context* context);


kqt_Context* kqt_new_Context(int buf_count,
                             uint32_t buf_size,
                             uint16_t voice_count,
                             uint8_t event_queue_size)
{
    if (buf_count <= 0)
    {
        return NULL;
    }
    if (buf_size <= 0)
    {
        return NULL;
    }
    if (voice_count <= 0)
    {
        return NULL;
    }
    if (event_queue_size <= 0)
    {
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
        return NULL;
    }
    context->song = NULL;
    context->play = NULL;
    context->play_silent = NULL;
    context->voices = NULL;

    context->voices = new_Voice_pool(voice_count, event_queue_size);
    if (context->voices == NULL)
    {
        kqt_del_Context(context);
        return NULL;
    }

    context->song = new_Song(buf_count, buf_size, event_queue_size);
    if (context->song == NULL)
    {
        kqt_del_Context(context);
        return NULL;
    }

    context->play = new_Playdata(44100, context->voices, Song_get_insts(context->song));
    if (context->play == NULL)
    {
        kqt_del_Context(context);
        return NULL;
    }
    context->play->order = Song_get_order(context->song);
    context->play->events = Song_get_events(context->song);

    context->play_silent = new_Playdata_silent(44100);
    if (context->play_silent == NULL)
    {
        kqt_del_Context(context);
        return NULL;
    }
    context->play_silent->order = Song_get_order(context->song);
    context->play_silent->events = Song_get_events(context->song);
    
    kqt_Context_stop(context);
    kqt_Context_set_position(context, NULL);
    return context;
}


char* kqt_Context_get_error(kqt_Context* context)
{
    if (context == NULL)
    {
        return NULL;
    }
    return context->error;
}


bool kqt_Context_load(kqt_Context* context, char* path)
{
    if (context == NULL)
    {
        return false;
    }
    if (path == NULL)
    {
        kqt_Context_set_error(context, "kqt_Context_load: path must not be NULL");
        return NULL;
    }
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        kqt_Context_set_error(context, "Couldn't access %s: %s", path, strerror(errno));
        return NULL;
    }
    File_tree* tree = NULL;
    Read_state* state = READ_STATE_AUTO;
    if (S_ISDIR(info->st_mode))
    {
        tree = new_File_tree_from_fs(path, state);
        if (tree == NULL)
        {
            kqt_Context_set_error(context, "%s:%d: %s",
                                  state->path, state->row, state->message);
            return NULL;
        }
    }
    else
    {
        tree = new_File_tree_from_tar(path, state);
        if (tree == NULL)
        {
            kqt_Context_set_error(context, "%s:%d: %s",
                                  state->path, state->row, state->message);
            return NULL;
        }
    }
    assert(tree != NULL);
    if (!Song_read(context->song, tree, state))
    {
        kqt_Context_set_error(context, "%s:%d: %s",
                              state->path, state->row, state->message);
        del_File_tree(tree);
        return NULL;
    }
    del_File_tree(tree);
    kqt_Context_stop(context);
    kqt_Context_set_position(context, NULL);
    return context;
}


uint64_t kqt_Context_get_length(kqt_Context* context, uint32_t freq)
{
    if (context == NULL)
    {
        return 0;
    }
    if (freq <= 0)
    {
        kqt_Context_set_error(context, "kqt_Context_get_length: freq must be positive");
        return 0;
    }
    kqt_Reltime_init(&context->play_silent->play_time);
    context->play_silent->play_frames = 0;
    kqt_Reltime_init(&context->play_silent->pos);
    context->play_silent->freq = freq;
    return Song_get_length(context->song, context->play_silent);
}


void kqt_Context_get_state(kqt_Context* context, kqt_Mix_state* mix_state)
{
    if (context == NULL || mix_state == NULL)
    {
        return;
    }
    Playdata* play = context->play;
    mix_state->playing = play->mode != STOP;
    mix_state->frames = play->play_frames;
    mix_state->subsong = play->subsong;
    mix_state->order = play->order_index;
    mix_state->pattern = play->pattern;
    kqt_Reltime_copy(&mix_state->pos, &play->pos);
    mix_state->tempo = play->tempo;
    mix_state->voices = play->active_voices;
    for (int i = 0; i < 2; ++i)
    {
        mix_state->min_amps[i] = play->min_amps[i];
        mix_state->max_amps[i] = play->max_amps[i];
        mix_state->clipped[i] = play->clipped[i];
    }
    Playdata_reset_stats(play);
    return;
}


int kqt_Context_get_buffer_count(kqt_Context* context)
{
    if (context == NULL)
    {
        return 0;
    }
    return Song_get_buf_count(context->song);
}


kqt_frame** kqt_Context_get_buffers(kqt_Context* context)
{
    if (context == NULL)
    {
        return NULL;
    }
    return Song_get_bufs(context->song);
}


bool kqt_Context_set_buffer_size(kqt_Context* context, uint32_t size)
{
    if (context == NULL)
    {
        return false;
    }
    if (size <= 0)
    {
        kqt_Context_set_error(context, "kqt_Context_set_buffer_size: size must be positive");
        return false;
    }
    bool success = Song_set_buf_size(context->song, size);
    if (!success)
    {
        kqt_Context_set_error(context, "Couldn't allocate memory for the new buffers");
        return false;
    }
    return true;
}


uint32_t kqt_Context_mix(kqt_Context* context, uint32_t nframes, uint32_t freq)
{
    assert(context != NULL);
    if (!context->play || context->song == NULL)
    {
        return 0;
    }
    if (freq == 0)
    {
        return 0;
    }
    context->play->freq = freq;
    return Song_mix(context->song, nframes, context->play);
}


bool kqt_Context_set_position(kqt_Context* context, char* position)
{
    if (context == NULL)
    {
        return false;
    }
    if (position == NULL)
    {
        position = "-1";
    }
    char* str = position;
    Read_state* state = READ_STATE_AUTO;
    int64_t subsong = -1;
    str = read_int(str, &subsong, state);
    if (state->error)
    {
        kqt_Context_set_error(context, "Invalid position indicator format");
        return false;
    }
    if (subsong < -1 || subsong >= SUBSONGS_MAX)
    {
        kqt_Context_set_error(context, "Subsong number %" PRId64 " is out of range");
        return false;
    }
    kqt_Context_stop(context);
    Playdata_reset_stats(context->play);
    Playdata_reset_stats(context->play_silent);
    if (subsong == -1)
    {
        context->play->mode = PLAY_SONG;
        Playdata_set_subsong(context->play, 0);
        context->play_silent->mode = PLAY_SONG;
        Playdata_set_subsong(context->play_silent, 0);
    }
    else
    {
        context->play->mode = PLAY_SUBSONG;
        Playdata_set_subsong(context->play, subsong);
        context->play_silent->mode = PLAY_SUBSONG;
        Playdata_set_subsong(context->play_silent, subsong);
    }
    return true;
}


#if 0
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
#endif


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


void kqt_Context_set_error(kqt_Context* context, char* message, ...)
{
    assert(context != NULL);
    assert(message != NULL);
    va_list args;
    va_start(args, message);
    vsnprintf(context->error, KQT_CONTEXT_ERROR_LENGTH, message, args);
    va_end(args);
    context->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    return;
}


void kqt_del_Context(kqt_Context* context)
{
    if (context == NULL)
    {
        return;
    }
    if (context->play_silent != NULL)
    {
        del_Playdata(context->play_silent);
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


