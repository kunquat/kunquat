

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

#include <Event_global.h>


void Event_global_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(EVENT_IS_GLOBAL(event->parent.type));
    assert(event->process != NULL);
    assert(play != NULL);
    event->process(event, play);
    return;
}


