

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


#ifndef K_EVENT_VOICE_SET_RESONANCE_H
#define K_EVENT_VOICE_SET_RESONANCE_H


#include <Event_voice.h>
#include <Reltime.h>


typedef struct Event_voice_set_resonance
{
    Event_voice parent;
    double resonance;
} Event_voice_set_resonance;


Event* new_Event_voice_set_resonance(Reltime* pos);


#endif // K_EVENT_VOICE_SET_RESONANCE_H


