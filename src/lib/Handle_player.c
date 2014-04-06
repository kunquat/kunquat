

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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
#include <string.h>
#include <math.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <Handle_private.h>
#include <kunquat/Player.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <module/Env_var.h>
#include <module/Module.h>
#include <string/common.h>


int kqt_Handle_play(kqt_Handle handle, long nframes)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (nframes <= 0)
    {
        Handle_set_error(
                h, ERROR_ARGUMENT, "Number of frames must be positive.");
        return 0;
    }

    Player_play(h->player, nframes);

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

    if (!Device_set_audio_rate(
                (Device*)h->module,
                Player_get_device_states(h->player),
                rate) ||
            !Player_set_audio_rate(h->player, rate))
    {
        Handle_set_error(h, ERROR_MEMORY,
                "Couldn't allocate memory after change of audio rate.");
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
        Handle_set_error(h, ERROR_ARGUMENT, "Buffer size must not be"
                " greater than %ld frames", KQT_AUDIO_BUFFER_SIZE_MAX);
        return 0;
    }

    if (!Device_set_buffer_size(
                (Device*)h->module,
                Player_get_device_states(h->player),
                size) ||
            !Player_set_audio_buffer_size(h->player, size))
    {
        Handle_set_error(h, ERROR_MEMORY,
                "Couldn't allocate memory for new buffers");
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
        Handle_set_error(h, ERROR_ARGUMENT,
                "Buffer #%d does not exist", index);
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
        Handle_set_error(h, ERROR_ARGUMENT,
                "Invalid track number: %d", track);
        return -1;
    }

    Player_reset(h->length_counter);
    Player_skip(h->length_counter, KQT_MAX_CALC_DURATION);

    return Player_get_nanoseconds(h->length_counter);
}


int kqt_Handle_set_position(
        kqt_Handle handle,
        int track,
        long long nanoseconds)
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
        Handle_set_error(
                h, ERROR_ARGUMENT, "nanoseconds must be non-negative");
        return 0;
    }

    int64_t skip_frames = ((double)nanoseconds / 1000000000L) *
        Player_get_audio_rate(h->player);

    Device_reset(
            (Device*)h->module,
            Player_get_device_states(h->player));

    Player_reset(h->player);
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


int kqt_Handle_fire_event(kqt_Handle handle, int channel, const char* event)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_data_is_validated(h, 0);

    if (channel < 0 || channel >= KQT_COLUMNS_MAX)
    {
        Handle_set_error(h, ERROR_ARGUMENT,
                "Invalid channel number: %d", channel);
        return 0;
    }
    if (event == NULL)
    {
        Handle_set_error(h, ERROR_ARGUMENT,
                "No event description given.");
        return 0;
    }

    Streader* sr = Streader_init(STREADER_AUTO, event, strlen(event));
    if (!Player_fire(h->player, channel, sr))
    {
        assert(Streader_is_error_set(sr));
        Handle_set_error(h, ERROR_ARGUMENT,
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


void Handle_stop(Handle* handle)
{
    assert(handle != NULL);

    Player_reset(handle->player);

#if 0
    handle->module->play_state->mode = STOP;
    Device_reset((Device*)handle->module);
#if 0
    Playdata_reset(handle->module->play_state);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(handle->module->channels[i]);
    }
#endif
    handle->module->play_state->track = 0;
    handle->module->play_state->tempo = NAN;
#endif
    return;
}


