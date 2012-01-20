

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CHANNEL_SET_ARPEGGIO_NOTE_H
#define K_EVENT_CHANNEL_SET_ARPEGGIO_NOTE_H


#include <stdbool.h>

#include <Event_channel.h>
#include <Reltime.h>
#include <Value.h>


Event* new_Event_channel_set_arpeggio_note(Reltime* pos);


bool Event_channel_set_arpeggio_note_process(Channel_state* ch_state,
                                             Value* value);


#endif // K_EVENT_CHANNEL_SET_ARPEGGIO_NOTE_H


