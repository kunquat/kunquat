

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


#ifndef K_EVENT_GLOBAL_SHIFT_SCALE_INTERVALS_H
#define K_EVENT_GLOBAL_SHIFT_SCALE_INTERVALS_H


#include <Event_global.h>
#include <Reltime.h>
#include <Playdata.h>


typedef struct Event_global_shift_scale_intervals
{
    Event_global parent;
//    int64_t new_ref;
//    int64_t fixed_point;
} Event_global_shift_scale_intervals;


Event* new_Event_global_shift_scale_intervals(Reltime* pos);


bool Event_global_shift_scale_intervals_process(Playdata* global_state, char* fields);


#endif // K_EVENT_GLOBAL_SHIFT_SCALE_INTERVALS_H


