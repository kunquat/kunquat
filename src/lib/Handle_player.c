

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


kqt_frame** kqt_Handle_get_buffers(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_buffers", NULL);
    return Song_get_bufs(handle->song);
}


long long kqt_Handle_get_duration(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_get_duration", 0);
    kqt_Reltime_init(&handle->play_silent->play_time);
    handle->play_silent->play_frames = 0;
    kqt_Reltime_init(&handle->play_silent->pos);
    handle->play_silent->freq = 1000000000;
    return Song_skip(handle->song, handle->play_silent, UINT64_MAX);
}


int kqt_Handle_seek_nanoseconds(kqt_Handle* handle, long long nanoseconds)
{
    check_handle(handle, "kqt_Handle_seek_nanoseconds", 0);
    if (nanoseconds < 0)
    {
        kqt_Handle_set_error(handle,
                "kqt_Handle_seek_nanoseconds: nanoseconds must not be negative");
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


long long kqt_Handle_tell_nanoseconds(kqt_Handle* handle)
{
    check_handle(handle, "kqt_Handle_tell_nanoseconds", 0);
    return ((long long)handle->play->play_frames * 1000000000L) / handle->play->freq;
}


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
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


