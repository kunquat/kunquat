

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


#ifndef K_EVENT_GLOBAL_SLIDE_TEMPO_H
#define K_EVENT_GLOBAL_SLIDE_TEMPO_H


#include <Event_global.h>
#include <Reltime.h>


typedef struct Event_global_slide_tempo
{
    Event_global parent;
    double target_tempo;
} Event_global_slide_tempo;


Event* new_Event_global_slide_tempo(Reltime* pos);


#endif // K_EVENT_GLOBAL_SLIDE_TEMPO_H


