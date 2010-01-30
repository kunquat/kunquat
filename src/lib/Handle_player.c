

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <Handle_private.h>
#include <kunquat/Player.h>
#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Event_handler.h>
#include <Voice_pool.h>

#include <xmemory.h>


long kqt_Handle_mix(kqt_Handle* handle, long nframes, long freq)
{
    check_handle(handle, 0);
    if (handle->song == NULL || !handle->song->play_state->mode)
    {
        return 0;
    }
    if (nframes <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Number of frames must"
                " be positive.");
        return 0;
    }
    if (freq <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Mixing frequency must"
                " be positive.");
        return 0;
    }
    handle->song->play_state->freq = freq;
    return Song_mix(handle->song, nframes, handle->song->event_handler);
}


int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size)
{
    check_handle(handle, 0);
    if (size <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Buffer size must be"
                " positive");
        return 0;
    }
    if (size > 4194304)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Buffer size must not be"
                " greater than 4194304 frames");
        return 0;
    }
    bool success = Song_set_buf_size(handle->song, size);
    if (!success)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory for new buffers");
        return 0;
    }
    return 1;
}


long kqt_Handle_get_buffer_size(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return Song_get_buf_size(handle->song);
}


int kqt_Handle_get_buffer_count(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return Song_get_buf_count(handle->song);
}


float* kqt_Handle_get_buffer(kqt_Handle* handle, int index)
{
    check_handle(handle, NULL);
    if (index < 0 || index >= Song_get_buf_count(handle->song))
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Buffer #%d does not exist", index);
        return NULL;
    }
    return (float*)Song_get_bufs(handle->song)[index];
}


long long kqt_Handle_get_duration(kqt_Handle* handle, int subsong)
{
    check_handle(handle, -1);
    if (subsong < -1 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid Subsong number: %d", subsong);
        return -1;
    }
    Reltime_init(&handle->song->skip_state->play_time);
    handle->song->skip_state->play_frames = 0;
    Playdata_reset(handle->song->skip_state);
    if (subsong == -1)
    {
        handle->song->skip_state->mode = PLAY_SONG;
        Playdata_set_subsong(handle->song->skip_state, 0);
    }
    else
    {
        handle->song->skip_state->mode = PLAY_SUBSONG;
        Playdata_set_subsong(handle->song->skip_state, subsong);
    }
    Reltime_init(&handle->song->skip_state->pos);
    handle->song->skip_state->freq = 1000000000;
    return Song_skip(handle->song, handle->song->skip_handler, UINT64_MAX);
}


int kqt_Handle_set_position(kqt_Handle* handle, int subsong, long long nanoseconds)
{
    check_handle(handle, 0);
    if (subsong < -1 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid Subsong number: %d", subsong);
        return 0;
    }
    if (nanoseconds < 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "nanoseconds must be non-negative");
        return 0;
    }
    char pos[32] = { '\0' };
    snprintf(pos, 32, "%d+%lld", subsong, nanoseconds);
    return kqt_Handle_set_position_desc(handle, pos);
}


long long kqt_Handle_get_position(kqt_Handle* handle)
{
    check_handle(handle, 0);
    return ((long long)handle->song->play_state->play_frames * 1000000000L) /
           handle->song->play_state->freq;
}


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
    handle->song->play_state->mode = STOP;
    Playdata_reset(handle->song->play_state);
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
    return;
}


