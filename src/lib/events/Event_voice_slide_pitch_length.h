

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


#ifndef K_EVENT_VOICE_SLIDE_PITCH_LENGTH_H
#define K_EVENT_VOICE_SLIDE_PITCH_LENGTH_H


#include <Event_voice.h>
#include <Reltime.h>


typedef struct Event_voice_slide_pitch_length
{
    Event_voice parent;
    Reltime length;
} Event_voice_slide_pitch_length;


Event* new_Event_voice_slide_pitch_length(Reltime* pos);


#endif // K_EVENT_VOICE_SLIDE_PITCH_LENGTH_H


