

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


#ifndef K_EVENT_VOICE_H
#define K_EVENT_VOICE_H


#include <Event.h>
#include <Voice.h>


typedef struct Event_voice
{
    Event parent;
    void (*process)(struct Event_voice* event, Voice* voice);
} Event_voice;


/**
 * Processes the Voice event.
 *
 * \param event   The Voice event -- must not be \c NULL.
 * \param voice   The Voice -- must not be \c NULL.
 */
void Event_voice_process(Event_voice* event, Voice* voice);


#endif // K_EVENT_VOICE_H


