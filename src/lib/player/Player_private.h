

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PLAYER_PRIVATE_H
#define K_PLAYER_PRIVATE_H


#include <stdbool.h>
#include <stdint.h>

#include <Environment.h>
#include <Event_handler.h>
#include <player/Cgiter.h>
#include <player/Channel.h>
#include <player/Device_states.h>
#include <player/Event_buffer.h>
#include <player/Master_params.h>
#include <player/Player.h>
#include <Voice_pool.h>


struct Player
{
    const Module* module;

    int32_t audio_rate;
    int32_t audio_buffer_size;
    float*  audio_buffers[2];
    int32_t audio_frames_available;

    Device_states* device_states;
    Environment*   env;
    Event_buffer*  event_buffer;
    Voice_pool*    voices;
    Master_params  master_params;
    Channel*       channels[KQT_CHANNELS_MAX];
    Event_handler* event_handler;

    double frame_remainder; // used for sub-frame time tracking

    bool cgiters_accessed;
    Cgiter cgiters[KQT_CHANNELS_MAX];

    // Position tracking
    int64_t audio_frames_processed;
    int64_t nanoseconds_history;

    bool events_returned;
};


#endif // K_PLAYER_PRIVATE_H


