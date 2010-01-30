

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


#ifndef K_EVENT_GLOBAL_JUMP_H
#define K_EVENT_GLOBAL_JUMP_H


#include <stdbool.h>
#include <stdint.h>

#include <Event_global.h>
#include <Reltime.h>
#include <Playdata.h>


typedef struct Event_global_jump
{
    Event_global parent;
    uint64_t play_id;
    int64_t counter;
    int16_t subsong;
    int16_t section;
    Reltime row;
} Event_global_jump;


Event* new_Event_global_jump(Reltime* pos);


#endif // K_EVENT_GLOBAL_JUMP_H


