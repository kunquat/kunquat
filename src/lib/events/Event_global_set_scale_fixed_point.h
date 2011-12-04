

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H
#define K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H


#include <stdbool.h>

#include <Event_global.h>
#include <Reltime.h>
#include <Playdata.h>


Event* new_Event_global_set_scale_fixed_point(Reltime* pos);


bool Event_global_set_scale_fixed_point_process(Playdata* global_state,
                                                char* fields);


#endif // K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H


