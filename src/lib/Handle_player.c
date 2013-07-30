

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <Env_var.h>
#include <Handle_private.h>
#include <kunquat/Player.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <Module.h>
#include <string_common.h>
#include <xassert.h>


long kqt_Handle_mix(kqt_Handle* handle, long nframes)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (nframes <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Number of frames must"
                " be positive.");
        return 0;
    }

    if (Player_has_stopped(handle->player)) // TODO: remove
        return 0;

    Player_play(handle->player, nframes);

    return Player_get_frames_available(handle->player); // TODO: remove
#if 0
    handle->module->play_state->freq =
            Device_get_mix_rate((Device*)handle->module);
    return Module_mix(handle->module, nframes, handle->module->event_handler);
#endif
}


int kqt_Handle_set_mixing_rate(kqt_Handle* handle, long rate)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (rate <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Mixing rate must be"
                " positive");
        return 0;
    }

    if (!Device_set_mix_rate((Device*)handle->module, rate) ||
            !Player_set_audio_rate(handle->player, rate))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory after change of mixing rate.");
        return 0;
    }

    return 1;
}


long kqt_Handle_get_mixing_rate(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    return Player_get_audio_rate(handle->player);
}


int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (size <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Buffer size must be"
                " positive");
        return 0;
    }
    if (size > KQT_AUDIO_BUFFER_SIZE_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Buffer size must not be"
                " greater than %ld frames", KQT_AUDIO_BUFFER_SIZE_MAX);
        return 0;
    }

    if (!Device_set_buffer_size((Device*)handle->module, size) ||
            !Player_set_audio_buffer_size(handle->player, size))
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
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    return Player_get_audio_buffer_size(handle->player);
}


int kqt_Handle_get_buffer_count(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    return KQT_BUFFERS_MAX;
}


const float* kqt_Handle_get_buffer(kqt_Handle* handle, int index)
{
    check_handle(handle, NULL);
    check_data_is_valid(handle, NULL);
    check_data_is_validated(handle, NULL);

    if (index < 0 || index >= KQT_BUFFERS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Buffer #%d does not exist", index);
        return NULL;
    }

    return Player_get_audio(handle->player, index);
}


long long kqt_Handle_get_duration(kqt_Handle* handle, int track)
{
    check_handle(handle, -1);
    check_data_is_valid(handle, -1);
    check_data_is_validated(handle, -1);

    if (track < -1 || track >= KQT_TRACKS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid track number: %d", track);
        return -1;
    }

    Player_reset(handle->length_counter);
    Player_skip(handle->length_counter, KQT_MAX_CALC_DURATION);

    return Player_get_nanoseconds(handle->length_counter);
}


int kqt_Handle_set_position(
        kqt_Handle* handle,
        int track,
        long long nanoseconds)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (track < -1 || track >= KQT_TRACKS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid track number: %d", track);
        return 0;
    }
    if (nanoseconds < 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "nanoseconds must be non-negative");
        return 0;
    }

    int64_t skip_frames = ((double)nanoseconds / 1000000000L) *
        Player_get_audio_rate(handle->player);

    Player_reset(handle->player);
    Player_skip(handle->player, skip_frames);

    return 1;
}


long long kqt_Handle_get_position(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    return Player_get_nanoseconds(handle->player);
}


int kqt_Handle_fire(kqt_Handle* handle, int channel, char* event)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (channel < 0 || channel >= KQT_COLUMNS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid channel number: %d", channel);
        return 0;
    }
    if (event == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "No event description given.");
        return 0;
    }

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(handle->player, channel, event, rs))
    {
        assert(rs->error);
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid event description `%s`: %s",
                event, rs->message);
        return 0;
    }

    return 1;
}


int kqt_Handle_receive(kqt_Handle* handle, char* dest, int size)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (dest == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "dest must not be NULL");
        return 0;
    }
    if (size <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "size must be positive");
        return 0;
    }

    const char* events = Player_get_events(handle->player);
    const int len = MIN((int)strlen(events) + 1, size);
    strncpy(dest, events, len);
    dest[len - 1] = '\0';

    return strlen(events) > 2;
}


int kqt_Handle_treceive(kqt_Handle* handle, char* dest, int size)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);

    if (dest == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "dest must not be NULL");
        return 0;
    }
    if (size <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "size must be positive");
        return 0;
    }

    const char* events = Player_get_events(handle->player);
    const int len = MIN((int)strlen(events) + 1, size);
    strncpy(dest, events, len);
    dest[len - 1] = '\0';

    return len > 3;
}


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));

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


