

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

#include <Event_common.h>
#include <memory.h>
#include <xassert.h>


Event* Event_init(
        Event* event,
        Tstamp* pos,
        Event_type type)
{
    assert(event != NULL);
    assert(pos != NULL);
    assert(EVENT_IS_VALID(type));
    event->type = type;
    Tstamp_copy(&event->pos, pos);
    event->desc = NULL;
    event->destroy = del_Event_default;
    return event;
}


void del_Event_default(Event* event)
{
    if (event == NULL)
    {
        return;
    }
    assert(EVENT_IS_VALID(event->type));
    memory_free(event->desc);
    memory_free(event);
    return;
}


