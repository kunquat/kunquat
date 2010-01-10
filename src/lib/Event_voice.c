

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>

#include <Event_voice.h>


void Event_voice_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(EVENT_IS_VOICE(event->parent.type));
    assert(voice != NULL);
    event->process(event, voice);
    return;
}


