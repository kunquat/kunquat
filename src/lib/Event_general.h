

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_GENERAL_H
#define K_EVENT_GENERAL_H


#include <Event_pg.h>


/**
 * Events that can be placed in any column.
 */
typedef struct Event_general
{
    Event_pg parent;
} Event_general;


#endif // K_EVENT_GENERAL_H


