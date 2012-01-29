

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CHANNEL_H
#define K_EVENT_CHANNEL_H


#include <Event_pg.h>
#include <Channel.h>


typedef struct Event_channel
{
    Event_pg parent;
//    void (*process)(struct Event_channel* event, Channel* ch);
} Event_channel;


/**
 * Processes the Channel event.
 *
 * \param event   The Channel event -- must not be \c NULL.
 * \param ch      The Channel to be affected -- must not be \c NULL.
 */
//void Event_channel_process(Event_channel* event, Channel* ch);


#endif // K_EVENT_CHANNEL_H


