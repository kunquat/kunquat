

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


#ifndef K_EVENT_CHANNEL_SET_PANNING_H
#define K_EVENT_CHANNEL_SET_PANNING_H


#include <Event_channel.h>
#include <Reltime.h>


Event* new_Event_channel_set_panning(Reltime* pos);


bool Event_channel_set_panning_process(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_SET_PANNING_H


