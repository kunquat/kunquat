

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <Event.h>
#include <kunquat/limits.h>
#include <xassert.h>


Tstamp* Event_get_pos(Event* event)
{
    assert(event != NULL);
    return &event->pos;
}


void Event_set_pos(Event* event, Tstamp* pos)
{
    assert(event != NULL);
    assert(pos != NULL);
    Tstamp_copy(&event->pos, pos);
    return;
}


Event_type Event_get_type(Event* event)
{
    assert(event != NULL);
    return event->type;
}


char* Event_get_desc(Event* event)
{
    assert(event != NULL);
    return event->desc;
}


void del_Event(Event* event)
{
    if (event == NULL)
    {
        return;
    }
    assert(event->destroy != NULL);
    event->destroy(event);
    return;
}


