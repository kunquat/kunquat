

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef K_EVENT_GLOBAL_RETUNE_SCALE_H
#define K_EVENT_GLOBAL_RETUNE_SCALE_H


#include <Event_global.h>
#include <Reltime.h>


typedef struct Event_global_retune_scale
{
    Event_global parent;
    int64_t scale_index;
    int64_t new_ref;
    int64_t fixed_point;
} Event_global_retune_scale;


Event* new_Event_global_retune_scale(Reltime* pos);


#endif // K_EVENT_GLOBAL_RETUNE_SCALE_H


