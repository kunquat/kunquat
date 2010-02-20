

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


#ifndef K_EVENT_GLOBAL_SLIDE_TEMPO_LENGTH_H
#define K_EVENT_GLOBAL_SLIDE_TEMPO_LENGTH_H


#include <Event_global.h>
#include <Reltime.h>


typedef struct Event_global_slide_tempo_length
{
    Event_global parent;
    Reltime length;
} Event_global_slide_tempo_length;


Event* new_Event_global_slide_tempo_length(Reltime* pos);


bool Event_global_slide_tempo_length_process(Playdata* global_state,
                                             char* fields);


#endif // K_EVENT_GLOBAL_SLIDE_TEMPO_LENGTH_H


