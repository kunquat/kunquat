

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


#ifndef K_EVENT_GLOBAL_H
#define K_EVENT_GLOBAL_H


#include <Event.h>
#include <Playdata.h>


typedef struct Event_global
{
    Event parent;
    void (*process)(struct Event_global* event, Playdata* play);
} Event_global;


/**
 * Processes the Global event.
 *
 * \param event   The Global event -- must not be \c NULL.
 * \param play    The Playdata -- must not be \c NULL.
 */
void Event_global_process(Event_global* event, Playdata* play);


#endif // K_EVENT_GLOBAL_H


