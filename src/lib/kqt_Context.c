

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
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <kunquat/Context.h>
#include <kunquat/Player_ext.h>

#include <Song_limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>
#include <File_base.h> // for string parsing

#include <xmemory.h>


#define KQT_CONTEXT_ERROR_LENGTH (256)
#define POSITION_LENGTH (64)


// For errors without an associated Kunquat Context.
static char null_error[KQT_CONTEXT_ERROR_LENGTH] = { '\0' };


struct kqt_Context
{
    Song* song;
    Playdata* play;
    Playdata* play_silent;
    Voice_pool* voices;
    char error[KQT_CONTEXT_ERROR_LENGTH];
    char position[POSITION_LENGTH];
};


void kqt_Context_set_error(kqt_Context* context, char* message, ...);

void kqt_Context_stop(kqt_Context* context);


kqt_Context* kqt_new_Context(int buf_count,
                             long buf_size,
                             short voice_count,
                             short event_queue_size)
{
    if (buf_count <= 0)
    {
        kqt_Context_set_error(NULL, "kqt_new_Context: buf_count must be positive");
        return NULL;
    }
    if (buf_size <= 0)
    {
        kqt_Context_set_error(NULL, "kqt_new_Context: buf_size must be positive");
        return NULL;
    }
    if (voice_count <= 0)
    {
        kqt_Context_set_error(NULL, "kqt_new_Context: voice_count must be positive");
        return NULL;
    }
    if (event_queue_size <= 0)
    {
        kqt_Context_set_error(NULL, "kqt_new_Context: event_queue_size must be positive");
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
        kqt_Context_set_error(NULL, "Couldn't allocate memory for a new Kunquat Context");
        return NULL;
    }
    context->song = NULL;
    context->play = NULL;
    context->play_silent = NULL;
    context->voices = NULL;
    context->error[0] = context->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    context->position[0] = context->position[POSITION_LENGTH - 1] = '\0';

    context->voices = new_Voice_pool(voice_count, event_queue_size);
    if (context->voices == NULL)
    {
        kqt_del_Context(context);
        kqt_Context_set_error(NULL, "Couldn't allocate memory for a new Kunquat Context");
        return NULL;
    }

    context->song = new_Song(buf_count, buf_size, event_queue_size);
    if (context->song == NULL)
    {
        kqt_del_Context(context);
        kqt_Context_set_error(NULL, "Couldn't allocate memory for a new Kunquat Context");
        return NULL;
    }

    context->play = new_Playdata(44100, context->voices, Song_get_insts(context->song));
    if (context->play == NULL)
    {
        kqt_del_Context(context);
        kqt_Context_set_error(NULL, "Couldn't allocate memory for a new Kunquat Context");
        return NULL;
    }
    context->play->order = Song_get_order(context->song);
    context->play->events = Song_get_events(context->song);

    context->play_silent = new_Playdata_silent(44100);
    if (context->play_silent == NULL)
    {
        kqt_del_Context(context);
        kqt_Context_set_error(NULL, "Couldn't allocate memory for a new Kunquat Context");
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
        return null_error;
    }
    return context->error;
}


int kqt_Context_load(kqt_Context* context, char* path)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_load: context must not be NULL");
        return 0;
    }
    if (path == NULL)
    {
        kqt_Context_set_error(context, "kqt_Context_load: path must not be NULL");
        return 0;
    }
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        kqt_Context_set_error(context, "Couldn't access %s: %s", path, strerror(errno));
        return 0;
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
            return 0;
        }
    }
    else
    {
        tree = new_File_tree_from_tar(path, state);
        if (tree == NULL)
        {
            kqt_Context_set_error(context, "%s:%d: %s",
                                  state->path, state->row, state->message);
            return 0;
        }
    }
    assert(tree != NULL);
    if (!Song_read(context->song, tree, state))
    {
        kqt_Context_set_error(context, "%s:%d: %s",
                              state->path, state->row, state->message);
        del_File_tree(tree);
        return 0;
    }
    del_File_tree(tree);
    kqt_Context_stop(context);
    kqt_Context_set_position(context, NULL);
    return 1;
}


int kqt_Context_get_subsong_length(kqt_Context* context, int subsong)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_subsong_length: context must not be NULL");
        return -1;
    }
    if (subsong < 0 || subsong >= SUBSONGS_MAX)
    {
        kqt_Context_set_error(context, "Invalid subsong number: %d", subsong);
        return -1;
    }
    assert(context->song != NULL);
    Subsong* ss = Order_get_subsong(Song_get_order(context->song), subsong);
    if (ss == NULL)
    {
        return 0;
    }
    return Subsong_get_length(ss);
}


long long kqt_Context_get_length_ns(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_length: context must not be NULL");
        return 0;
    }
    kqt_Reltime_init(&context->play_silent->play_time);
    context->play_silent->play_frames = 0;
    kqt_Reltime_init(&context->play_silent->pos);
    context->play_silent->freq = 1000000000;
    return Song_skip(context->song, context->play_silent, UINT64_MAX);
}


int kqt_Context_get_buffer_count(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_buffer_count: context must not be NULL");
        return 0;
    }
    return Song_get_buf_count(context->song);
}


kqt_frame** kqt_Context_get_buffers(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_buffers: context must not be NULL");
        return NULL;
    }
    return Song_get_bufs(context->song);
}


int kqt_Context_set_buffer_size(kqt_Context* context, long size)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_set_buffer_size: context must not be NULL");
        return 0;
    }
    if (size <= 0)
    {
        kqt_Context_set_error(context, "kqt_Context_set_buffer_size: size must be positive");
        return 0;
    }
    bool success = Song_set_buf_size(context->song, size);
    if (!success)
    {
        kqt_Context_set_error(context, "Couldn't allocate memory for the new buffers");
        return 0;
    }
    return 1;
}


long kqt_Context_get_buffer_size(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_buffer_size: context must not be NULL");
        return 0;
    }
    return Song_get_buf_size(context->song);
}


int kqt_unwrap_time(char* time,
                    int* subsong,
                    int* section,
                    long long* beats,
                    long* remainder,
                    long long* nanoseconds)
{
    if (time == NULL)
    {
        return 0;
    }
    long read_subsong = -1;
    long read_section = 0;
    long long read_beats = 0;
    long read_remainder = 0;
    long long read_ns = 0;
    char* next = NULL;
    read_subsong = strtol(time, &next, 0);
    if (read_subsong < -1 || read_subsong >= SUBSONGS_MAX)
    {
        return 0;
    }
    time = next;
    next = NULL;
    if (*time == '/')
    {
        ++time;
        read_section = strtol(time, &next, 0);
        if (read_section < 0 || read_section >= ORDERS_MAX)
        {
            return 0;
        }
        time = next;
        next = NULL;
    }
    if (*time == '/')
    {
        ++time;
        read_beats = strtoll(time, &next, 0);
        if (read_beats < 0 || read_beats == LLONG_MAX)
        {
            return 0;
        }
        time = next;
        if (*time != ':')
        {
            return 0;
        }
        ++time;
        next = NULL;
        read_remainder = strtol(time, &next, 0);
        if (read_remainder < 0 || read_remainder >= KQT_RELTIME_BEAT)
        {
            return 0;
        }
        time = next;
        next = NULL;
    }
    if (*time == '+')
    {
        ++time;
        read_ns = strtoll(time, &next, 0);
        if (read_ns < 0 || read_ns == LLONG_MAX)
        {
            return 0;
        }
        time = next;
        next = NULL;
    }
    if (*time != '\0')
    {
        return 0;
    }
    if (subsong != NULL)
    {
        *subsong = read_subsong;
    }
    if (section != NULL)
    {
        *section = read_section;
    }
    if (beats != NULL)
    {
        *beats = read_beats;
    }
    if (remainder != NULL)
    {
        *remainder = read_remainder;
    }
    if (nanoseconds != NULL)
    {
        *nanoseconds = read_ns;
    }
    return 1;
}


int kqt_Context_set_position_ns(kqt_Context* context, long long nanoseconds)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_set_position_ns: context must not be NULL");
        return 0;
    }
    if (nanoseconds < 0)
    {
        kqt_Context_set_error(NULL, "kqt_Context_set_position_ns: nanoseconds must not be negative");
        return 0;
    }
    char pos[32] = { '\0' };
    int subsong = -1;
    if (context->play->mode == PLAY_SUBSONG)
    {
        subsong = context->play->subsong;
    }
    snprintf(pos, 32, "%d+%lld", subsong, nanoseconds);
    return kqt_Context_set_position(context, pos);
}


int kqt_Context_set_position(kqt_Context* context, char* position)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_set_position: context must not be NULL");
        return 0;
    }
    if (position == NULL)
    {
        position = "-1";
    }
    char* str = position;
    
    int subsong = -1;
    int section = 0;
    long long beats = 0;
    long remainder = 0;
    long long nanoseconds = 0;
    if (!kqt_unwrap_time(str, &subsong, &section, &beats, &remainder, &nanoseconds))
    {
        kqt_Context_set_error(context, "Invalid position indication: %s", position);
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
    context->play->order_index = section;
    context->play_silent->order_index = section;
    kqt_Reltime_set(&context->play->pos, beats, remainder);
    kqt_Reltime_set(&context->play_silent->pos, beats, remainder);
    context->play->play_frames = 0;
    context->play_silent->play_frames = 0;
    if (nanoseconds > 0)
    {
        uint64_t frame_skip = ((double)nanoseconds / 1000000000) * context->play->freq;
        Song_skip(context->song, context->play, frame_skip);
        kqt_Reltime_copy(&context->play_silent->pos, &context->play->pos);
    }
    return 1;
}


void kqt_Context_reset_stats(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_reset_stats: context must not be NULL");
        return;
    }
    Playdata_reset_stats(context->play);
    Playdata_reset_stats(context->play_silent);
    return;
}


long long kqt_Context_get_position_ns(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_position_ns: context must not be NULL");
        return false;
    }
    return ((long long)context->play->play_frames * 1000000000L) / context->play->freq;
}


long kqt_Context_mix(kqt_Context* context, long nframes, long freq)
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


int kqt_Context_end_reached(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_end_reached: context must not be NULL");
        return 1;
    }
    return context->play->mode == STOP;
}


long long kqt_Context_get_frames_mixed(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_frames_mixed: context must not be NULL");
        return 0;
    }
    return context->play->play_frames;
}


char* kqt_Context_get_position(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_position: context must not be NULL");
        return NULL;
    }
    snprintf(context->position, POSITION_LENGTH, "%d/%d/%lld:%ld",
             context->play->mode == PLAY_SONG ? -1 : (int)context->play->subsong,
             (int)context->play->order_index,
             (long long)kqt_Reltime_get_beats(&context->play->pos),
             (long)kqt_Reltime_get_rem(&context->play->pos));
    return context->position;
}


double kqt_Context_get_tempo(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_tempo: context must not be NULL");
        return 0;
    }
    return context->play->tempo;
}


int kqt_Context_get_voice_count(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_Context_get_voice_count: context must not be NULL");
        return 0;
    }
    return context->play->active_voices;
}


double kqt_Context_get_min_amplitude(kqt_Context* context, int buffer)
{
    if (context == NULL || buffer < 0 || buffer >= BUF_COUNT_MAX)
    {
        return INFINITY;
    }
    return context->play->min_amps[buffer];
}


double kqt_Context_get_max_amplitude(kqt_Context* context, int buffer)
{
    if (context == NULL || buffer < 0 || buffer >= BUF_COUNT_MAX)
    {
        return -INFINITY;
    }
    return context->play->max_amps[buffer];
}


long kqt_Context_get_clipped(kqt_Context* context, int buffer)
{
    if (context == NULL || buffer < 0 || buffer >= BUF_COUNT_MAX)
    {
        return 0;
    }
    return context->play->clipped[buffer];
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
    assert(message != NULL);
    char* error = null_error;
    if (context != NULL)
    {
        error = context->error;
    }
    va_list args;
    va_start(args, message);
    vsnprintf(error, KQT_CONTEXT_ERROR_LENGTH, message, args);
    va_end(args);
    context->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    return;
}


void kqt_del_Context(kqt_Context* context)
{
    if (context == NULL)
    {
        kqt_Context_set_error(NULL, "kqt_del_Context: context must not be NULL");
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


