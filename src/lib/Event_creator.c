

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <Event.h>
#include <Event_type.h>
#include <Event_global_set_tempo.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>


typedef Event* (*Event_cons)(Reltime* pos);


Event* new_Event(Event_type type, Reltime* pos)
{
    assert(EVENT_TYPE_IS_VALID(type));
    assert(pos != NULL);
    static bool cons_initialised = false;
    static Event_cons cons[EVENT_TYPE_LAST] = { NULL };
    if (!cons_initialised)
    {
        cons[EVENT_TYPE_GLOBAL_SET_TEMPO] = new_Event_global_set_tempo;
        cons[EVENT_TYPE_NOTE_ON] = new_Event_voice_note_on;
        cons[EVENT_TYPE_NOTE_OFF] = new_Event_voice_note_off;
        cons_initialised = true;
    }
    if (cons[type] == NULL)
    {
        return NULL; // XXX: should we consider the caller broken in this case?
    }
    return cons[type](pos);
}


