

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


#include <debug/assert.h>
#include <player/Channel_event_buffer.h>

#include <stdlib.h>
#include <string.h>


void Channel_event_buffer_init(Channel_event_buffer* buffer)
{
    rassert(buffer != NULL);

    buffer->event_count = 0;

    return;
}


int Channel_event_buffer_get_event_count(const Channel_event_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->event_count;
}


void Channel_event_buffer_add_event(
        Channel_event_buffer* buffer, const Channel_event* event)
{
    rassert(buffer != NULL);
    rassert(buffer->event_count < CHANNEL_EVENTS_MAX);

    memcpy(&buffer->events[buffer->event_count], event, sizeof(Channel_event));
    ++buffer->event_count;

    return;
}


const Channel_event* Channel_event_buffer_get_event(
        const Channel_event_buffer* buffer, int index)
{
    rassert(buffer != NULL);
    rassert(index >= 0);
    rassert(index < buffer->event_count);

    return &buffer->events[index];
}


