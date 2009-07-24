

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
#include <stdint.h>
#include <limits.h>

#include <Handle_private.h>
#include <kunquat/Player_ext.h>
#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>

#include <xmemory.h>


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


int kqt_Handle_set_position_desc(kqt_Handle* handle, char* position)
{
    check_handle(handle, "kqt_Handle_set_position_desc", 0);
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


char* kqt_Handle_get_position_desc(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_position_desc", NULL);
    snprintf(handle->position, POSITION_LENGTH, "%d/%d/%lld:%ld",
             handle->play->mode == PLAY_SONG ? -1 : (int)handle->play->subsong,
             (int)handle->play->order_index,
             (long long)kqt_Reltime_get_beats(&handle->play->pos),
             (long)kqt_Reltime_get_rem(&handle->play->pos));
    return handle->position;
}


int kqt_Handle_end_reached(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_end_reached", 1);
    return handle->play->mode == STOP;
}


long long kqt_Handle_get_frames_mixed(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_frames_mixed", 0);
    return handle->play->play_frames;
}


double kqt_Handle_get_tempo(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_tempo", 0);
    return handle->play->tempo;
}


int kqt_Handle_get_voice_count(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_voice_count", 0);
    return handle->play->active_voices;
}


double kqt_Handle_get_min_amplitude(kqt_Handle* handle, int buffer)
{
    check_handle(handle, "kqt_Handle_get_min_amplitude", INFINITY);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return INFINITY;
    }
    return handle->play->min_amps[buffer];
}


double kqt_Handle_get_max_amplitude(kqt_Handle* handle, int buffer)
{
    check_handle(handle, "kqt_Handle_get_max_amplitude", -INFINITY);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return -INFINITY;
    }
    return handle->play->max_amps[buffer];
}


long kqt_Handle_get_clipped(kqt_Handle* handle, int buffer)
{
    check_handle(handle, "kqt_Handle_get_clipped", 0);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
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


void kqt_Handle_reset_stats(kqt_Handle* handle)
{
    check_handle_void(handle, "kqt_Handle_reset_stats");
    Playdata_reset_stats(handle->play);
    Playdata_reset_stats(handle->play_silent);
    return;
}


