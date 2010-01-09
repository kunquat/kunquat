

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_EVENT_VOICE_H
#define K_EVENT_VOICE_H


#include <Event.h>
#include <Voice.h>


typedef struct Event_voice
{
    Event parent;
    void (*process)(struct Event_voice* event, Voice* voice);
} Event_voice;


/**
 * Processes the Voice event.
 *
 * \param event   The Voice event -- must not be \c NULL.
 * \param voice   The Voice -- must not be \c NULL.
 */
void Event_voice_process(Event_voice* event, Voice* voice);


#endif // K_EVENT_VOICE_H


