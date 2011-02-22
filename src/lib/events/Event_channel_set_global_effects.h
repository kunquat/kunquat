

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


#ifndef K_EVENT_CHANNEL_SET_GLOBAL_EFFECTS_H
#define K_EVENT_CHANNEL_SET_GLOBAL_EFFECTS_H


#include <Event_channel.h>
#include <Reltime.h>


Event* new_Event_channel_set_global_effects(Reltime* pos);


bool Event_channel_set_global_effects_process(Channel_state* ch_state,
                                              char* fields);


#endif // K_EVENT_CHANNEL_SET_GLOBAL_EFFECTS_H


