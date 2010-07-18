

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


#ifndef K_EVENT_CHANNEL_SET_GEN_BOOL_H
#define K_EVENT_CHANNEL_SET_GEN_BOOL_H


#include <Event_channel.h>
#include <Reltime.h>


typedef struct Event_channel_set_gen_bool
{
    Event_channel parent;
} Event_channel_set_gen_bool;


Event* new_Event_channel_set_gen_bool(Reltime* pos);

bool Event_channel_set_gen_bool_process(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_SET_GEN_BOOL_H


