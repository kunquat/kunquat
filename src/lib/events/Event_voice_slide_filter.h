

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


#ifndef K_EVENT_VOICE_SLIDE_FILTER_H
#define K_EVENT_VOICE_SLIDE_FILTER_H


#include <Event_voice.h>
#include <Reltime.h>


typedef struct Event_voice_slide_filter
{
    Event_voice parent;
    double target_cutoff;
} Event_voice_slide_filter;


Event* new_Event_voice_slide_filter(Reltime* pos);


#endif // K_EVENT_VOICE_SLIDE_FILTER_H


