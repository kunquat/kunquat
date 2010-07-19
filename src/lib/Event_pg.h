

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


#ifndef K_EVENT_PG_H
#define K_EVENT_PG_H


#include <Event.h>


/**
 * This class is used for "pseudo-global" events which need to be processed
 * at a global level but really affect only part of the playback, i.e.
 * Instrument, Generator and DSP events.
 */
typedef struct Event_pg
{
    Event parent;
    int ch_index;
} Event_pg;


#endif // K_EVENT_PG_H


