

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <memory.h>
#include <player/events/Event_common.h>
#include <xassert.h>


#if 0
Event* Event_init(
        Event* event,
        Tstamp* pos,
        Event_type type)
{
    assert(event != NULL);
    assert(pos != NULL);
    assert(Event_is_valid(type));
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
    assert(Event_is_valid(event->type));
    memory_free(event->desc);
    memory_free(event);
    return;
}
#endif


