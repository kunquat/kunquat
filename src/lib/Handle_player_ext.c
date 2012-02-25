

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include <Handle_private.h>
#include <kunquat/Player_ext.h>
#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>
#include <xassert.h>
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
    check_handle(handle, 0);
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
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid position indication: %s", position);
        return false;
    }

    kqt_Handle_stop(handle);
    Playdata_reset_stats(handle->song->play_state);
    Playdata_reset_stats(handle->song->skip_state);
    if (subsong == -1)
    {
        handle->song->play_state->mode = PLAY_SONG;
        Playdata_set_subsong(handle->song->play_state, 0, true);
        handle->song->skip_state->mode = PLAY_SONG;
        Playdata_set_subsong(handle->song->skip_state, 0, true);
    }
    else
    {
        handle->song->play_state->mode = PLAY_SUBSONG;
        Playdata_set_subsong(handle->song->play_state, subsong, true);
        handle->song->skip_state->mode = PLAY_SUBSONG;
        Playdata_set_subsong(handle->song->skip_state, subsong, true);
    }
    handle->song->play_state->section = section;
    handle->song->skip_state->section = section;
    Reltime_set(&handle->song->play_state->pos, beats, remainder);
    Reltime_set(&handle->song->skip_state->pos, beats, remainder);
    handle->song->play_state->play_frames = 0;
    handle->song->skip_state->play_frames = 0;
    if (nanoseconds > 0)
    {
        uint64_t frame_skip = ((double)nanoseconds / 1000000000) * handle->song->play_state->freq;
        Song_skip(handle->song, handle->song->event_handler, frame_skip);
        Reltime_copy(&handle->song->skip_state->pos, &handle->song->play_state->pos);
    }
    return 1;
}


char* kqt_Handle_get_position_desc(kqt_Handle* handle)
{
    check_handle(handle, NULL);
    snprintf(handle->position, POSITION_LENGTH, "%d/%d/%lld:%ld",
             handle->song->play_state->mode == PLAY_SONG ? -1 : (int)handle->song->play_state->subsong,
             (int)handle->song->play_state->section,
             (long long)Reltime_get_beats(&handle->song->play_state->pos),
             (long)Reltime_get_rem(&handle->song->play_state->pos));
    return handle->position;
}


int kqt_Handle_end_reached(kqt_Handle* handle)
{
    check_handle(handle, 1);
    return handle->song->play_state->mode == STOP;
}


long long kqt_Handle_get_frames_mixed(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return handle->song->play_state->play_frames;
}


double kqt_Handle_get_tempo(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return handle->song->play_state->tempo;
}


int kqt_Handle_get_voice_count(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return handle->song->play_state->active_voices;
}


double kqt_Handle_get_min_amplitude(kqt_Handle* handle, int buffer)
{
    check_handle(handle, INFINITY);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return INFINITY;
    }
    return handle->song->play_state->min_amps[buffer];
}


double kqt_Handle_get_max_amplitude(kqt_Handle* handle, int buffer)
{
    check_handle(handle, -INFINITY);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return -INFINITY;
    }
    return handle->song->play_state->max_amps[buffer];
}


long long kqt_Handle_get_clipped(kqt_Handle* handle, int buffer)
{
    check_handle(handle, 0);
    if (buffer < 0 || buffer >= KQT_BUFFERS_MAX)
    {
        return 0;
    }
    return handle->song->play_state->clipped[buffer];
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
    handle->song->play_state->pattern = num;
    handle->song->play_state->tempo = tempo;
    Playdata_reset_stats(handle->song->play_state);
    handle->song->play_state->mode = PLAY_PATTERN;
    return;
}


void kqt_Handle_play_subsong(kqt_Handle* handle, uint16_t subsong)
{
    assert(handle != NULL);
    assert(subsong < KQT_SUBSONGS_MAX);
    kqt_Handle_stop(handle);
    handle->play->subsong = subsong;
    Subsong* ss = Subsong_table_get(handle->play->subsongs, handle->play->subsong);
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
    check_handle_void(handle);
    Playdata_reset_stats(handle->song->play_state);
    Playdata_reset_stats(handle->song->skip_state);
    return;
}


