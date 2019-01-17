

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CHANNEL_EVENT_BUFFER_H
#define KQT_CHANNEL_EVENT_BUFFER_H


#include <decl.h>
#include <player/Event_type.h>
#include <Value.h>

#include <stdint.h>
#include <stdlib.h>


#define CHANNEL_EVENTS_MAX 24576


typedef struct Channel_event
{
    int32_t frame_offset;
    Event_type type;
    Value argument;
} Channel_event;


struct Channel_event_buffer
{
    int event_count;
    Channel_event events[CHANNEL_EVENTS_MAX];
};


void Channel_event_buffer_init(Channel_event_buffer* buffer);


int Channel_event_buffer_get_event_count(const Channel_event_buffer* buffer);


void Channel_event_buffer_add_event(
        Channel_event_buffer* buffer, const Channel_event* event);


const Channel_event* Channel_event_buffer_get_event(
        const Channel_event_buffer* buffer, int index);


#endif // KQT_CHANNEL_EVENT_BUFFER_H


