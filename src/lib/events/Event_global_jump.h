

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


#ifndef K_EVENT_GLOBAL_JUMP_H
#define K_EVENT_GLOBAL_JUMP_H


#include <stdbool.h>
#include <stdint.h>

#include <AAtree.h>
#include <transient/Master_params.h>
#include <Tstamp.h>
#include <Playdata.h>


typedef struct Event_global_jump
{
    Event parent;
    AAtree* counters;
    AAiter* counters_iter;
    //uint64_t play_id;
    //int64_t counter;
    //int16_t subsong;
    //int16_t section;
    //Tstamp row;
} Event_global_jump;


Event* new_Event_global_jump(Tstamp* pos);


void Trigger_global_jump_process(Event* event, Master_params* master_params, Playdata* play);


bool Trigger_global_jump_set_locations(
        Event_global_jump* event,
        AAtree* locations,
        AAiter* locations_iter);


#endif // K_EVENT_GLOBAL_JUMP_H


