

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
#include <Module.h>
#include <Playdata.h>
#include <Event_handler.h>
#include <string_common.h>
#include <Voice_pool.h>
#include <xassert.h>


long kqt_Handle_mix(kqt_Handle* handle, long nframes)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);
    if (handle->module == NULL || !handle->module->play_state->mode)
    {
        return 0;
    }
    if (nframes <= 0)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Number of frames must"
                " be positive.");
        return 0;
    }
    handle->module->play_state->freq =
            Device_get_mix_rate((Device*)handle->module);
    return Module_mix(handle->module, nframes, handle->module->event_handler);
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
    if (!Device_set_mix_rate((Device*)handle->module, rate))
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
    return Device_get_mix_rate((Device*)handle->module);
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
    if (size > KQT_BUFFER_SIZE_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Buffer size must not be"
                " greater than %ld frames", KQT_BUFFER_SIZE_MAX);
        return 0;
    }
    if (!Device_set_buffer_size((Device*)handle->module, size))
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
    return Device_get_buffer_size((Device*)handle->module);
}


int kqt_Handle_get_buffer_count(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);
    return KQT_BUFFERS_MAX;
}


float* kqt_Handle_get_buffer(kqt_Handle* handle, int index)
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
    Audio_buffer* buffer = Device_get_buffer(&handle->module->parent,
                                   DEVICE_PORT_TYPE_RECEIVE, 0);
    assert(buffer != NULL);
    return (float*)Audio_buffer_get_buffer(buffer, index);
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
    Tstamp_init(&handle->module->skip_state->play_time);
    handle->module->skip_state->play_frames = 0;
    Playdata_reset(handle->module->skip_state);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(handle->module->channels[i]);
    }
    if (track == -1)
    {
        handle->module->skip_state->mode = PLAY_SONG;
        Playdata_set_track(handle->module->skip_state, 0, true);
    }
    else
    {
        handle->module->skip_state->mode = PLAY_SUBSONG;
        Playdata_set_track(handle->module->skip_state, track, true);
    }
    Tstamp_init(&handle->module->skip_state->pos);
    handle->module->skip_state->freq = 1000000000;
    return Module_skip(handle->module, handle->module->skip_handler,
                     KQT_MAX_CALC_DURATION);
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

    kqt_Handle_stop(handle);
    Playdata_reset_stats(handle->module->play_state);
    Playdata_reset_stats(handle->module->skip_state);
    if (track == -1)
    {
        handle->module->play_state->mode = PLAY_SONG;
        Playdata_set_track(handle->module->play_state, 0, true);
        handle->module->skip_state->mode = PLAY_SONG;
        Playdata_set_track(handle->module->skip_state, 0, true);
    }
    else
    {
        handle->module->play_state->mode = PLAY_SUBSONG;
        Playdata_set_track(handle->module->play_state, track, true);
        handle->module->skip_state->mode = PLAY_SUBSONG;
        Playdata_set_track(handle->module->skip_state, track, true);
    }
    handle->module->play_state->system = 0;
    handle->module->skip_state->system = 0;
    Tstamp_set(&handle->module->play_state->pos, 0, 0);
    Tstamp_set(&handle->module->skip_state->pos, 0, 0);
    handle->module->play_state->play_frames = 0;
    handle->module->skip_state->play_frames = 0;
    if (nanoseconds > 0)
    {
        uint64_t frame_skip = ((double)nanoseconds / 1000000000) *
            handle->module->play_state->freq;
        Module_skip(handle->module, handle->module->event_handler, frame_skip);
        Tstamp_copy(
                &handle->module->skip_state->pos,
                &handle->module->play_state->pos);
    }
    return 1;
}


long long kqt_Handle_get_position(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_data_is_validated(handle, 0);
    return ((long long)handle->module->play_state->play_frames * 1000000000L) /
           handle->module->play_state->freq;
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

    // Set tempo if we haven't mixed anything yet
    Playdata* global_state = handle->module->play_state;
    if (isnan(global_state->tempo))
    {
        global_state->tempo = 120;
        const uint16_t track_index = global_state->track;
        const Track_list* tl = handle->module->track_list;
        if (tl != NULL && track_index < Track_list_get_len(tl))
        {
            int16_t song_index = Track_list_get_song_index(
                    tl, global_state->track);
            const bool existent = Subsong_table_get_existent(
                    global_state->subsongs, song_index);
            Subsong* ss = Subsong_table_get(
                    global_state->subsongs, song_index);
            if (existent && ss != NULL)
                global_state->tempo = Subsong_get_tempo(ss);
        }
    }

    Read_state* rs = READ_STATE_AUTO;
    const bool success = Event_handler_trigger_const(
            handle->module->event_handler,
            channel,
            event,
            false,
            rs);
    if (!success)
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
    return Event_handler_receive(handle->module->event_handler, dest, size);
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
    return Event_handler_treceive(handle->module->event_handler, dest, size);
}


#if 0
int kqt_Handle_get_state(kqt_Handle* handle,
                         char* key,
                         char* dest,
                         int size)
{
    check_handle(handle, 0);
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "key must not be NULL");
        return 0;
    }
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
    if (string_has_prefix(key, "env/"))
    {
        Env_var* var = Environment_get(handle->module->env,
                                       key + strlen("env/"));
        if (var == NULL)
        {
            return 0;
        }
        Env_var_get_value_json(var, dest, size);
        return 1;
    }
    return Playdata_get_state_value(handle->module->play_state,
                                    key, dest, size);
}
#endif


void kqt_Handle_stop(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
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
    return;
}


