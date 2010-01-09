

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
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


