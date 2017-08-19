

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Handle_private.h>

#include <debug/assert.h>
#include <Error.h>
#include <init/Env_var.h>
#include <init/Module.h>
#include <kunquat/Player.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <string/common.h>

#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


int kqt_Handle_play(kqt_Handle handle, long nframes)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (nframes <= 0)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Number of frames must be positive.");
        return 0;
    }

    Player_play(h->player, (int32_t)min(nframes, KQT_AUDIO_BUFFER_SIZE_MAX));

    return 1;
}


int kqt_Handle_has_stopped(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_has_stopped(h->player);
}


long kqt_Handle_get_frames_available(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_frames_available(h->player);
}


int kqt_Handle_set_audio_rate(kqt_Handle handle, long rate)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (rate <= 0)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Audio rate must be positive");
        return 0;
    }
#if LONG_MAX > INT32_MAX
    else if ((int64_t)rate > INT32_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Audio rate must be <= %" PRId32, INT32_MAX);
        return 0;
    }
#endif

    if (!Player_set_audio_rate(h->player, (int32_t)rate))
    {
        Handle_set_error(
                h, ERROR_MEMORY, "Couldn't allocate memory after change of audio rate.");
        return 0;
    }

    return 1;
}


long kqt_Handle_get_audio_rate(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_audio_rate(h->player);
}


int kqt_Handle_set_thread_count(kqt_Handle handle, int count)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (count < 1)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Thread count must be positive");
        return 0;
    }
    if (count > KQT_THREADS_MAX)
    {
        Handle_set_error(
                h, ERROR_ARGUMENT, "Thread count must not exceed %d", KQT_THREADS_MAX);
        return 0;
    }

    Error* error = ERROR_AUTO;

    if (!Player_set_thread_count(h->player, count, error))
    {
        Handle_set_error_from_Error(h, error);
        return 0;
    }

    return 1;
}


int kqt_Handle_get_thread_count(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_thread_count(h->player);
}


int kqt_Handle_set_audio_buffer_size(kqt_Handle handle, long size)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (size <= 0)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Buffer size must be positive");
        return 0;
    }
    if (size > KQT_AUDIO_BUFFER_SIZE_MAX)
    {
        Handle_set_error(
                h,
                ERROR_ARGUMENT,
                "Buffer size must not be greater than %ld frames",
                KQT_AUDIO_BUFFER_SIZE_MAX);
        return 0;
    }

    if (!Player_set_audio_buffer_size(h->player, (int32_t)size))
    {
        Handle_set_error(h, ERROR_MEMORY, "Couldn't allocate memory for new buffers");
        return 0;
    }

    return 1;
}


long kqt_Handle_get_audio_buffer_size(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_audio_buffer_size(h->player);
}


#if 0
int kqt_Handle_get_buffer_count(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return KQT_BUFFERS_MAX;
}
#endif


const float* kqt_Handle_get_audio(kqt_Handle handle, int index)
{
    check_handle(handle, NULL);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, NULL);
    check_data_is_validated(h, NULL);

    if (index < 0 || index >= KQT_BUFFERS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Buffer #%d does not exist", index);
        return NULL;
    }

    return Player_get_audio(h->player, index);
}


long long kqt_Handle_get_duration(kqt_Handle handle, int track)
{
    check_handle(handle, -1);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, -1);
    check_data_is_validated(h, -1);

    if (track < -1 || track >= KQT_TRACKS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Invalid track number: %d", track);
        return -1;
    }

    Player_reset(h->length_counter, track);
    Player_skip(h->length_counter, KQT_CALC_DURATION_MAX);

    return Player_get_nanoseconds(h->length_counter);
}


int kqt_Handle_set_position(kqt_Handle handle, int track, long long nanoseconds)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (track < -1 || track >= KQT_TRACKS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Invalid track number: %d", track);
        return 0;
    }
    if (nanoseconds < 0)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "nanoseconds must be non-negative");
        return 0;
    }

    int64_t skip_frames = (int64_t)(((double)nanoseconds / 1000000000L) *
        Player_get_audio_rate(h->player));

    Device_states_reset(Player_get_device_states(h->player));

    Player_reset(h->player, track);
    Player_skip(h->player, skip_frames);

    return 1;
}


long long kqt_Handle_get_position(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_nanoseconds(h->player);
}


int kqt_Handle_set_channel_mute(kqt_Handle handle, int channel, int mute)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (channel < 0 || channel >= KQT_COLUMNS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Invalid channel number: %d", channel);
        return 0;
    }

    if (mute != 0 && mute != 1)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Invalid mute state: %d", mute);
        return 0;
    }

    Player_set_channel_mute(h->player, channel, mute);

    return 1;
}


int kqt_Handle_fire_event(kqt_Handle handle, int channel, const char* event)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (channel < 0 || channel >= KQT_COLUMNS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Invalid channel number: %d", channel);
        return 0;
    }
    if (event == NULL)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "No event description given");
        return 0;
    }

    const size_t length = strlen(event);
    if (length > 4096)
    {
        Handle_set_error(h, ERROR_ARGUMENT, "Event description is too long");
        return 0;
    }

    Streader* sr = Streader_init(STREADER_AUTO, event, (int64_t)length);
    if (!Player_fire(h->player, channel, sr))
    {
        rassert(Streader_is_error_set(sr));
        Handle_set_error(
                h,
                ERROR_ARGUMENT,
                "Invalid event description `%s`: %s",
                event, Streader_get_error_desc(sr));
        return 0;
    }

    return 1;
}


const char* kqt_Handle_receive_events(kqt_Handle handle)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    return Player_get_events(h->player);
}


