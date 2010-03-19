

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

#include <Event_channel.h>


#if 0
void Event_channel_process(Event_channel* event, Channel* ch)
{
    assert(event != NULL);
    assert(EVENT_IS_CHANNEL(event->parent.type));
    assert(event->process != NULL);
    assert(ch != NULL);
    event->process(event, ch);
    return;
}
#endif


