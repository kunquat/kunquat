

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


#ifndef K_EVENT_CHANNEL_H
#define K_EVENT_CHANNEL_H


#include <Event.h>
#include <Channel.h>


typedef struct Event_channel
{
    Event parent;
    void (*process)(struct Event_channel* event, Channel* ch);
} Event_channel;


/**
 * Processes the Channel event.
 *
 * \param event   The Channel event -- must not be \c NULL.
 * \param ch      The Channel to be affected -- must not be \c NULL.
 */
void Event_channel_process(Event_channel* event, Channel* ch);


#endif // K_EVENT_CHANNEL_H


