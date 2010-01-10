

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


#ifndef K_EVENT_VOICE_SET_PANNING_H
#define K_EVENT_VOICE_SET_PANNING_H


#include <Event_voice.h>
#include <Reltime.h>


typedef struct Event_voice_set_panning
{
    Event_voice parent;
    double panning;
} Event_voice_set_panning;


Event* new_Event_voice_set_panning(Reltime* pos);


#endif // K_EVENT_VOICE_SET_PANNING_H


