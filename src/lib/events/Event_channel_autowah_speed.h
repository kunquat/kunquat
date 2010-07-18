

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


#ifndef K_EVENT_CHANNEL_AUTOWAH_SPEED_H
#define K_EVENT_CHANNEL_AUTOWAH_SPEED_H


#include <Event_channel.h>
#include <Reltime.h>


typedef struct Event_channel_autowah_speed
{
    Event_channel parent;
//    double speed;
} Event_channel_autowah_speed;


Event* new_Event_channel_autowah_speed(Reltime* pos);


bool Event_channel_autowah_speed_process(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_AUTOWAH_SPEED_H


