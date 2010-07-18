

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


#ifndef K_EVENT_CHANNEL_NOTE_ON_H
#define K_EVENT_CHANNEL_NOTE_ON_H


#include <stdint.h>

#include <Event_channel.h>
#include <Reltime.h>


typedef struct Event_channel_note_on
{
    Event_channel parent;
//    double cents;
} Event_channel_note_on;


Event* new_Event_channel_note_on(Reltime* pos);

bool Event_channel_note_on_process(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_NOTE_ON_H


