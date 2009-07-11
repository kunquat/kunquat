

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

#include <kunquat/Handle.h>
#include <kunquat/Player_ext.h>

#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>

#include <xmemory.h>


#define KQT_CONTEXT_ERROR_LENGTH (256)
#define POSITION_LENGTH (64)


// For errors without an associated Kunquat Handle.
static char null_error[KQT_CONTEXT_ERROR_LENGTH] = { '\0' };


struct kqt_Handle
{
    Song* song;
    Playdata* play;
    Playdata* play_silent;
    Voice_pool* voices;
    char error[KQT_CONTEXT_ERROR_LENGTH];
    char position[POSITION_LENGTH];
};


void kqt_Handle_set_error(kqt_Handle* handle, char* message, ...);

void kqt_Handle_stop(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle(long buffer_size)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, "kqt_new_Handle: buf_size must be positive");
        return NULL;
    }
    kqt_Handle* handle = xalloc(kqt_Handle);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }
    handle->song = NULL;
    handle->play = NULL;
    handle->play_silent = NULL;
    handle->voices = NULL;
    handle->error[0] = handle->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    handle->position[0] = handle->position[POSITION_LENGTH - 1] = '\0';

    int buffer_count = 2;
    int voice_count = 256;
    int event_queue_size = 32;

    handle->voices = new_Voice_pool(voice_count, event_queue_size);
    if (handle->voices == NULL)
    {
        kqt_del_Handle(handle);
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }

    handle->song = new_Song(buffer_count, buffer_size, event_queue_size);
    if (handle->song == NULL)
    {
        kqt_del_Handle(handle);
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }

    handle->play = new_Playdata(44100, handle->voices, Song_get_insts(handle->song));
    if (handle->play == NULL)
    {
        kqt_del_Handle(handle);
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }
    handle->play->order = Song_get_order(handle->song);
    handle->play->events = Song_get_events(handle->song);

    handle->play_silent = new_Playdata_silent(44100);
    if (handle->play_silent == NULL)
    {
        kqt_del_Handle(handle);
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }
    handle->play_silent->order = Song_get_order(handle->song);
    handle->play_silent->events = Song_get_events(handle->song);
    
    kqt_Handle_stop(handle);
    kqt_Handle_set_position(handle, NULL);
    return handle;
}


char* kqt_Handle_get_error(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        return null_error;
    }
    return handle->error;
}


int kqt_Handle_load(kqt_Handle* handle, char* path)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_load: handle must not be NULL");
        return 0;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(handle, "kqt_Handle_load: path must not be NULL");
        return 0;
    }
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        kqt_Handle_set_error(handle, "Couldn't access %s: %s", path, strerror(errno));
        return 0;
    }
    File_tree* tree = NULL;
    Read_state* state = READ_STATE_AUTO;
    if (S_ISDIR(info->st_mode))
    {
        tree = new_File_tree_from_fs(path, state);
        if (tree == NULL)
        {
            kqt_Handle_set_error(handle, "%s:%d: %s",
                                  state->path, state->row, state->message);
            return 0;
        }
    }
    else
    {
        tree = new_File_tree_from_tar(path, state);
        if (tree == NULL)
        {
            kqt_Handle_set_error(handle, "%s:%d: %s",
                                  state->path, state->row, state->message);
            return 0;
        }
    }
    assert(tree != NULL);
    if (!Song_read(handle->song, tree, state))
    {
        kqt_Handle_set_error(handle, "%s:%d: %s",
                              state->path, state->row, state->message);
        del_File_tree(tree);
        return 0;
    }
    del_File_tree(tree);
    kqt_Handle_stop(handle);
    kqt_Handle_set_position(handle, NULL);
    return 1;
}


int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_subsong_length: handle must not be NULL");
        return -1;
    }
    if (subsong < 0 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle, "Invalid subsong number: %d", subsong);
        return -1;
    }
    assert(handle->song != NULL);
    Subsong* ss = Order_get_subsong(Song_get_order(handle->song), subsong);
    if (ss == NULL)
    {
        return 0;
    }
    return Subsong_get_length(ss);
}


long long kqt_Handle_get_duration(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_duration: handle must not be NULL");
        return 0;
    }
    kqt_Reltime_init(&handle->play_silent->play_time);
    handle->play_silent->play_frames = 0;
    kqt_Reltime_init(&handle->play_silent->pos);
    handle->play_silent->freq = 1000000000;
    return Song_skip(handle->song, handle->play_silent, UINT64_MAX);
}


int kqt_Handle_get_buffer_count(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_buffer_count: handle must not be NULL");
        return 0;
    }
    return Song_get_buf_count(handle->song);
}


kqt_frame** kqt_Handle_get_buffers(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_buffers: handle must not be NULL");
        return NULL;
    }
    return Song_get_bufs(handle->song);
}


int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_set_buffer_size: handle must not be NULL");
        return 0;
    }
    if (size <= 0)
    {
        kqt_Handle_set_error(handle, "kqt_Handle_set_buffer_size: size must be positive");
        return 0;
    }
    bool success = Song_set_buf_size(handle->song, size);
    if (!success)
    {
        kqt_Handle_set_error(handle, "Couldn't allocate memory for the new buffers");
        return 0;
    }
    return 1;
}


long kqt_Handle_get_buffer_size(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_buffer_size: handle must not be NULL");
        return 0;
    }
    return Song_get_buf_size(handle->song);
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
    if (read_subsong < -1 || read_subsong >= KQT_SUBSONGS_MAX)
    {
        return 0;
    }
    time = next;
    next = NULL;
    if (*time == '/')
    {
        ++time;
        read_section = strtol(time, &next, 0);
        if (read_section < 0 || read_section >= KQT_SECTIONS_MAX)
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


int kqt_Handle_seek_nanoseconds(kqt_Handle* handle, long long nanoseconds)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_set_position_ns: handle must not be NULL");
        return 0;
    }
    if (nanoseconds < 0)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_set_position_ns: nanoseconds must not be negative");
        return 0;
    }
    char pos[32] = { '\0' };
    int subsong = -1;
    if (handle->play->mode == PLAY_SUBSONG)
    {
        subsong = handle->play->subsong;
    }
    snprintf(pos, 32, "%d+%lld", subsong, nanoseconds);
    return kqt_Handle_set_position(handle, pos);
}


int kqt_Handle_set_position(kqt_Handle* handle, char* position)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_set_position: handle must not be NULL");
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
        kqt_Handle_set_error(handle, "Invalid position indication: %s", position);
        return false;
    }
    
    kqt_Handle_stop(handle);
    Playdata_reset_stats(handle->play);
    Playdata_reset_stats(handle->play_silent);
    if (subsong == -1)
    {
        handle->play->mode = PLAY_SONG;
        Playdata_set_subsong(handle->play, 0);
        handle->play_silent->mode = PLAY_SONG;
        Playdata_set_subsong(handle->play_silent, 0);
    }
    else
    {
        handle->play->mode = PLAY_SUBSONG;
        Playdata_set_subsong(handle->play, subsong);
        handle->play_silent->mode = PLAY_SUBSONG;
        Playdata_set_subsong(handle->play_silent, subsong);
    }
    handle->play->order_index = section;
    handle->play_silent->order_index = section;
    kqt_Reltime_set(&handle->play->pos, beats, remainder);
    kqt_Reltime_set(&handle->play_silent->pos, beats, remainder);
    handle->play->play_frames = 0;
    handle->play_silent->play_frames = 0;
    if (nanoseconds > 0)
    {
        uint64_t frame_skip = ((double)nanoseconds / 1000000000) * handle->play->freq;
        Song_skip(handle->song, handle->play, frame_skip);
        kqt_Reltime_copy(&handle->play_silent->pos, &handle->play->pos);
    }
    return 1;
}


void kqt_Handle_reset_stats(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_reset_stats: handle must not be NULL");
        return;
    }
    Playdata_reset_stats(handle->play);
    Playdata_reset_stats(handle->play_silent);
    return;
}


long long kqt_Handle_tell_nanoseconds(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_position_ns: handle must not be NULL");
        return false;
    }
    return ((long long)handle->play->play_frames * 1000000000L) / handle->play->freq;
}


long kqt_Handle_mix(kqt_Handle* handle, long nframes, long freq)
{
    assert(handle != NULL);
    if (!handle->play || handle->song == NULL)
    {
        return 0;
    }
    if (freq == 0)
    {
        return 0;
    }
    handle->play->freq = freq;
    return Song_mix(handle->song, nframes, handle->play);
}


int kqt_Handle_end_reached(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_end_reached: handle must not be NULL");
        return 1;
    }
    return handle->play->mode == STOP;
}


long long kqt_Handle_get_frames_mixed(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_frames_mixed: handle must not be NULL");
        return 0;
    }
    return handle->play->play_frames;
}


char* kqt_Handle_get_position(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_position: handle must not be NULL");
        return NULL;
    }
    snprintf(handle->position, POSITION_LENGTH, "%d/%d/%lld:%ld",
             handle->play->mode == PLAY_SONG ? -1 : (int)handle->play->subsong,
             (int)handle->play->order_index,
             (long long)kqt_Reltime_get_beats(&handle->play->pos),
             (long)kqt_Reltime_get_rem(&handle->play->pos));
    return handle->position;
}


double kqt_Handle_get_tempo(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_tempo: handle must not be NULL");
        return 0;
    }
    return handle->play->tempo;
}


int kqt_Handle_get_voice_count(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_Handle_get_voice_count: handle must not be NULL");
        return 0;
    }
    return handle->play->active_voices;
}


double kqt_Handle_get_min_amplitude(kqt_Handle* handle, int buffer)
{
    if (handle == NULL || buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return INFINITY;
    }
    return handle->play->min_amps[buffer];
}


double kqt_Handle_get_max_amplitude(kqt_Handle* handle, int buffer)
{
    if (handle == NULL || buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return -INFINITY;
    }
    return handle->play->max_amps[buffer];
}


long kqt_Handle_get_clipped(kqt_Handle* handle, int buffer)
{
    if (handle == NULL || buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return 0;
    }
    return handle->play->clipped[buffer];
}


#if 0
void kqt_Handle_play_pattern(kqt_Handle* handle, int16_t num, double tempo)
{
    assert(handle != NULL);
    assert(num >= 0);
    assert(num < KQT_PATTERNS_MAX);
    assert(isfinite(tempo));
    assert(tempo > 0);
    kqt_Handle_stop(handle);
    handle->play->pattern = num;
    handle->play->tempo = tempo;
    Playdata_reset_stats(handle->play);
    handle->play->mode = PLAY_PATTERN;
    return;
}


void kqt_Handle_play_subsong(kqt_Handle* handle, uint16_t subsong)
{
    assert(handle != NULL);
    assert(subsong < KQT_SUBSONGS_MAX);
    kqt_Handle_stop(handle);
    handle->play->subsong = subsong;
    Subsong* ss = Order_get_subsong(handle->play->order, handle->play->subsong);
    if (ss == NULL)
    {
        handle->play->tempo = 120;
    }
    else
    {
        handle->play->tempo = Subsong_get_tempo(ss);
    }
    Playdata_reset_stats(handle->play);
    handle->play->mode = PLAY_SONG;
    return;
}


void kqt_Handle_play_song(kqt_Handle* handle)
{
    assert(handle != NULL);
    kqt_Handle_stop(handle);
    handle->play->subsong = Song_get_subsong(handle->song);
    Playdata_reset_stats(handle->play);
    handle->play->mode = PLAY_SONG;
    return;
}


void kqt_Handle_play_event(kqt_Handle* handle)
{
    assert(handle != NULL);
    if (handle->play->mode >= PLAY_EVENT)
    {
        return;
    }
    kqt_Handle_stop(handle);
    Playdata_reset_stats(handle->play);
    handle->play->mode = PLAY_EVENT;
    return;
}
#endif


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle != NULL);
    handle->play->mode = STOP;
    Voice_pool_reset(handle->voices);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(handle->play->channels[i]);
    }
    kqt_Reltime_init(&handle->play->play_time);
    handle->play->play_frames = 0;
    handle->play->subsong = Song_get_subsong(handle->song);
    Subsong* ss = Order_get_subsong(handle->play->order, handle->play->subsong);
    if (ss == NULL)
    {
        handle->play->tempo = 120;
    }
    else
    {
        handle->play->tempo = Subsong_get_tempo(ss);
    }
    handle->play->order_index = 0;
    handle->play->pattern = 0;
    kqt_Reltime_init(&handle->play->pos);
    return;
}


void kqt_Handle_set_error(kqt_Handle* handle, char* message, ...)
{
    assert(message != NULL);
    char* error = null_error;
    if (handle != NULL)
    {
        error = handle->error;
    }
    va_list args;
    va_start(args, message);
    vsnprintf(error, KQT_CONTEXT_ERROR_LENGTH, message, args);
    va_end(args);
    handle->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    return;
}


void kqt_del_Handle(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_del_Handle: handle must not be NULL");
        return;
    }
    if (handle->play_silent != NULL)
    {
        del_Playdata(handle->play_silent);
    }
    if (handle->play != NULL)
    {
        del_Playdata(handle->play);
    }
    if (handle->voices != NULL)
    {
        del_Voice_pool(handle->voices);
    }
    if (handle->song != NULL)
    {
        del_Song(handle->song);
    }
    xfree(handle);
    return;
}


