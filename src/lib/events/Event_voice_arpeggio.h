

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef K_EVENT_VOICE_ARPEGGIO_H
#define K_EVENT_VOICE_ARPEGGIO_H


#include <Event_voice.h>
#include <Reltime.h>
#include <kunquat/limits.h>


typedef struct Event_voice_arpeggio
{
    Event_voice parent;
    double speed;
    int64_t notes[KQT_ARPEGGIO_NOTES_MAX];
} Event_voice_arpeggio;


Event* new_Event_voice_arpeggio(Reltime* pos);


#endif // K_EVENT_VOICE_ARPEGGIO_H


