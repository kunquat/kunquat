

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


#ifndef K_EVENT_CHANNEL_ARPEGGIO_H
#define K_EVENT_CHANNEL_ARPEGGIO_H


#include <Event_channel.h>
#include <Reltime.h>
#include <kunquat/limits.h>


typedef struct Event_channel_arpeggio
{
    Event_channel parent;
    double speed;
    int64_t notes[KQT_ARPEGGIO_NOTES_MAX];
} Event_channel_arpeggio;


Event* new_Event_channel_arpeggio(Reltime* pos);


bool Event_channel_arpeggio_handle(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_ARPEGGIO_H


