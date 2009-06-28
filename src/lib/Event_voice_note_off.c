

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

#include <Event_voice_note_off.h>

#include <xmemory.h>


static Event_field_desc note_off_desc[] =
{
    {
        .type = EVENT_FIELD_TYPE_NONE
    }
};


static bool Event_voice_note_off_set(Event* event, int index, void* data);

static void* Event_voice_note_off_get(Event* event, int index);

static void del_Event_voice_note_off(Event* event);

static void Event_voice_note_off_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_note_off(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_note_off* event = xalloc(Event_voice_note_off);
    if (event == NULL)
    {
        return NULL;
    }
    event->parent.parent.type = EVENT_TYPE_NOTE_OFF;
    event->parent.parent.field_types = note_off_desc;
    event->parent.parent.set = Event_voice_note_off_set;
    event->parent.parent.get = Event_voice_note_off_get;
    event->parent.parent.destroy = del_Event_voice_note_off;
    Reltime_copy(&event->parent.parent.pos, pos);
    event->parent.process = Event_voice_note_off_process;
    return (Event*)event;
}


static void Event_voice_note_off_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_TYPE_NOTE_OFF);
    assert(voice != NULL);
    voice->state.generic.note_on = false;
    return;
}


static bool Event_voice_note_off_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_OFF);
    assert(data != NULL);
    (void)event;
    (void)index;
    (void)data;
    return false;
}


static void* Event_voice_note_off_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_OFF);
    (void)event;
    (void)index;
    return NULL;
}


static void del_Event_voice_note_off(Event* event)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_OFF);
    xfree(event);
    return;
}


