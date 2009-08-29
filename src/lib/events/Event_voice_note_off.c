

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

#include <Event_common.h>
#include <Event_voice_note_off.h>

#include <xmemory.h>


static Event_field_desc note_off_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_note_off_set(Event* event, int index, void* data);

static void* Event_voice_note_off_get(Event* event, int index);

static void Event_voice_note_off_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_note_off,
                         EVENT_VOICE_NOTE_OFF,
                         note_off_desc,
                         (void)0)


static void Event_voice_note_off_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_NOTE_OFF);
    assert(voice != NULL);
    (void)event;
    voice->state.generic.note_on = false;
    return;
}


static bool Event_voice_note_off_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_NOTE_OFF);
    assert(data != NULL);
    (void)event;
    (void)index;
    (void)data;
    return false;
}


static void* Event_voice_note_off_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_NOTE_OFF);
    (void)event;
    (void)index;
    return NULL;
}


