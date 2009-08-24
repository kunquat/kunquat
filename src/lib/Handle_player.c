

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

#include <Handle_private.h>
#include <kunquat/Player.h>
#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>

#include <xmemory.h>


long kqt_Handle_mix(kqt_Handle* handle, long nframes, long freq)
{
    check_handle(handle, "kqt_Handle_mix", 0);
    if (handle->song == NULL || !handle->song->play_state->mode)
    {
        return 0;
    }
    if (freq == 0)
    {
        return 0;
    }
    handle->song->play_state->freq = freq;
    return Song_mix(handle->song, nframes, handle->song->play_state);
}


int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size)
{
    check_handle(handle, "kqt_Handle_set_buffer_size", 0);
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
    check_handle(handle, "kqt_Handle_get_buffer_size", 0);
    return Song_get_buf_size(handle->song);
}


int kqt_Handle_get_buffer_count(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_buffer_count", 0);
    return Song_get_buf_count(handle->song);
}


kqt_frame* kqt_Handle_get_buffer(kqt_Handle* handle, int index)
{
    check_handle(handle, "kqt_Handle_get_buffer", NULL);
    if (index < 0 || index >= Song_get_buf_count(handle->song))
    {
        kqt_Handle_set_error(handle,
                "kqt_Handle_get_buffer: buffer #%d doesn't exist", index);
        return NULL;
    }
    return Song_get_bufs(handle->song)[index];
}


long long kqt_Handle_get_duration(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_duration", 0);
    Reltime_init(&handle->song->skip_state->play_time);
    handle->song->skip_state->play_frames = 0;
    Reltime_init(&handle->song->skip_state->pos);
    handle->song->skip_state->freq = 1000000000;
    return Song_skip(handle->song, handle->song->skip_state, UINT64_MAX);
}


int kqt_Handle_set_position(kqt_Handle* handle, int subsong, long long nanoseconds)
{
    check_handle(handle, "kqt_Handle_set_position", 0);
    if (subsong < -1 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle,
                "kqt_Handle_seek: Invalid Subsong number: %d", subsong);
        return 0;
    }
    if (nanoseconds < 0)
    {
        kqt_Handle_set_error(handle,
                "kqt_Handle_seek: nanoseconds must not be negative");
        return 0;
    }
    char pos[32] = { '\0' };
    snprintf(pos, 32, "%d+%lld", subsong, nanoseconds);
    return kqt_Handle_set_position_desc(handle, pos);
}


long long kqt_Handle_get_position(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_position", 0);
    return ((long long)handle->song->play_state->play_frames * 1000000000L) /
           handle->song->play_state->freq;
}


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
    handle->song->play_state->mode = STOP;
    Voice_pool_reset(handle->song->play_state->voice_pool);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(handle->song->play_state->channels[i]);
    }
    Reltime_init(&handle->song->play_state->play_time);
    Reltime_init(&handle->song->play_state->delay_left);
    handle->song->play_state->play_frames = 0;
    handle->song->play_state->subsong = Song_get_subsong(handle->song);
    Subsong* ss = Subsong_table_get(handle->song->play_state->subsongs,
                                    handle->song->play_state->subsong);
    if (ss == NULL)
    {
        handle->song->play_state->tempo = 120;
    }
    else
    {
        handle->song->play_state->tempo = Subsong_get_tempo(ss);
    }
    handle->song->play_state->section = 0;
    handle->song->play_state->pattern = 0;
    Reltime_init(&handle->song->play_state->pos);
    return;
}


