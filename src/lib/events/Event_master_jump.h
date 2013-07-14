

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


#ifndef K_EVENT_MASTER_JUMP_H
#define K_EVENT_MASTER_JUMP_H


#include <stdbool.h>
#include <stdint.h>

#include <AAtree.h>
#include <Event.h>
#include <player/Master_params.h>
#include <Tstamp.h>


typedef struct Event_master_jump
{
    Event parent;
    AAtree* counters;
    AAiter* counters_iter;
    //uint64_t play_id;
    //int64_t counter;
    //Tstamp row;
} Event_master_jump;


Event* new_Event_master_jump(Tstamp* pos);


void Trigger_master_jump_process(Event* event, Master_params* master_params);


bool Trigger_master_jump_set_locations(
        Event_master_jump* event,
        AAtree* locations,
        AAiter* locations_iter);


#endif // K_EVENT_MASTER_JUMP_H


