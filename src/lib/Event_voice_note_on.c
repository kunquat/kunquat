

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

#include <Event_voice_note_on.h>
#include <kunquat/Reltime.h>
#include <Voice.h>
#include <Note_table.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc note_on_desc[] =
{
    {
        .type = EVENT_FIELD_TYPE_NOTE,
        .range.integral_type = { 0, KQT_SCALE_NOTES - 1 }
    },
    {
        .type = EVENT_FIELD_TYPE_NOTE_MOD,
        .range.integral_type = { -1, KQT_SCALE_NOTE_MODS - 1 }
    },
    {
        .type = EVENT_FIELD_TYPE_INT,
        .range.integral_type = { KQT_SCALE_OCTAVE_FIRST, KQT_SCALE_OCTAVE_LAST }
    },
    {
        .type = EVENT_FIELD_TYPE_INT,
        .range.integral_type = { 0, KQT_INSTRUMENTS_MAX }
    },
    {
        .type = EVENT_FIELD_TYPE_NONE
    }
};


static bool Event_voice_note_on_set(Event* event, int index, void* data);


static void* Event_voice_note_on_get(Event* event, int index);


static void del_Event_voice_note_on(Event* event);


static void Event_voice_note_on_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_note_on(kqt_Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_note_on* event = xalloc(Event_voice_note_on);
    if (event == NULL)
    {
        return NULL;
    }
    kqt_Reltime_copy(&event->parent.parent.pos, pos);
    event->parent.parent.type = EVENT_TYPE_NOTE_ON;
    event->parent.parent.field_types = note_on_desc;
    event->parent.parent.set = Event_voice_note_on_set;
    event->parent.parent.get = Event_voice_note_on_get;
    event->parent.parent.destroy = del_Event_voice_note_on;
    event->parent.process = Event_voice_note_on_process;
    event->note = 0;
    event->mod = -1;
    event->octave = KQT_SCALE_MIDDLE_OCTAVE;
    event->instrument = 1;
    return (Event*)event;
}


static void Event_voice_note_on_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_TYPE_NOTE_ON);
    assert(voice != NULL);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    Generator_process_note(voice->gen,
                           &voice->state.generic,
                           note_on->note,
                           note_on->mod,
                           note_on->octave);
    return;
}


static bool Event_voice_note_on_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_ON);
    assert(data != NULL);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    int64_t* num = (int64_t*)data;
    switch (index)
    {
        case 0:
        {
            Event_check_integral_range(*num, event->field_types[0]);
            note_on->note = *num;
            return true;
        }
        break;
        case 1:
        {
            Event_check_integral_range(*num, event->field_types[1]);
            note_on->mod = *num;
            return true;
        }
        break;
        case 2:
        {
            Event_check_integral_range(*num, event->field_types[2]);
            note_on->octave = *num;
            return true;
        }
        break;
        case 3:
        {
            Event_check_integral_range(*num, event->field_types[3]);
            note_on->instrument = *num;
            return true;
        }
        break;
        default:
        break;
    }
    return false;
}


static void* Event_voice_note_on_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_ON);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    switch (index)
    {
        case 0:
        {
            return &note_on->note;
        }
        break;
        case 1:
        {
            return &note_on->mod;
        }
        break;
        case 2:
        {
            return &note_on->octave;
        }
        break;
        case 3:
        {
            return &note_on->instrument;
        }
        break;
        default:
        break;
    }
    return NULL;
}


static void del_Event_voice_note_on(Event* event)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_NOTE_ON);
    xfree(event);
    return;
}


