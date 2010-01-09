

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_EVENT_CHANNEL_SET_INSTRUMENT_H
#define K_EVENT_CHANNEL_SET_INSTRUMENT_H


#include <Event_channel.h>
#include <Reltime.h>


typedef struct Event_channel_set_instrument
{
    Event_channel parent;
    int64_t instrument;
} Event_channel_set_instrument;


Event* new_Event_channel_set_instrument(Reltime* pos);


#endif // K_EVENT_CHANNEL_SET_INSTRUMENT_H


