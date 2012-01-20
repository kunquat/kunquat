

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CHANNEL_SET_GEN_FLOAT_H
#define K_EVENT_CHANNEL_SET_GEN_FLOAT_H


#include <Event_channel.h>
#include <Reltime.h>
#include <Value.h>


Event* new_Event_channel_set_gen_float(Reltime* pos);


bool Event_channel_set_gen_float_process(Channel_state* ch_state,
                                         Value* value);


#endif // K_EVENT_CHANNEL_SET_GEN_FLOAT_H


