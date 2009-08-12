

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <Event.h>
#include <Event_type.h>

#include <Event_global_set_tempo.h>

#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>

#include <Event_voice_set_force.h>
#include <Event_voice_slide_force.h>
#include <Event_voice_tremolo_speed.h>
#include <Event_voice_tremolo_depth.h>
#include <Event_voice_tremolo_delay.h>

#include <Event_voice_slide_pitch.h>
#include <Event_voice_vibrato_speed.h>
#include <Event_voice_vibrato_depth.h>
#include <Event_voice_vibrato_delay.h>
#include <Event_voice_arpeggio.h>

#include <Event_voice_set_filter.h>
#include <Event_voice_slide_filter.h>
#include <Event_voice_autowah_speed.h>
#include <Event_voice_autowah_depth.h>
#include <Event_voice_autowah_delay.h>
#include <Event_voice_set_resonance.h>

#include <Event_voice_set_panning.h>
#include <Event_voice_slide_panning.h>


typedef Event* (*Event_cons)(Reltime* pos);


Event* new_Event(Event_type type, Reltime* pos)
{
    assert(EVENT_IS_VALID(type));
    assert(pos != NULL);
    static bool cons_initialised = false;
    static Event_cons cons[EVENT_LAST] = { NULL };
    if (!cons_initialised)
    {
        cons[EVENT_GLOBAL_SET_TEMPO] = new_Event_global_set_tempo;

        cons[EVENT_VOICE_NOTE_ON] = new_Event_voice_note_on;
        cons[EVENT_VOICE_NOTE_OFF] = new_Event_voice_note_off;

        cons[EVENT_VOICE_SET_FORCE] = new_Event_voice_set_force;
        cons[EVENT_VOICE_SLIDE_FORCE] = new_Event_voice_slide_force;
        cons[EVENT_VOICE_TREMOLO_SPEED] = new_Event_voice_tremolo_speed;
        cons[EVENT_VOICE_TREMOLO_DEPTH] = new_Event_voice_tremolo_depth;
        cons[EVENT_VOICE_TREMOLO_DELAY] = new_Event_voice_tremolo_delay;

        cons[EVENT_VOICE_SLIDE_PITCH] = new_Event_voice_slide_pitch;
        cons[EVENT_VOICE_VIBRATO_SPEED] = new_Event_voice_vibrato_speed;
        cons[EVENT_VOICE_VIBRATO_DEPTH] = new_Event_voice_vibrato_depth;
        cons[EVENT_VOICE_VIBRATO_DELAY] = new_Event_voice_vibrato_delay;
        cons[EVENT_VOICE_ARPEGGIO] = new_Event_voice_arpeggio;

        cons[EVENT_VOICE_SET_FILTER] = new_Event_voice_set_filter;
        cons[EVENT_VOICE_SLIDE_FILTER] = new_Event_voice_slide_filter;
        cons[EVENT_VOICE_AUTOWAH_SPEED] = new_Event_voice_autowah_speed;
        cons[EVENT_VOICE_AUTOWAH_DEPTH] = new_Event_voice_autowah_depth;
        cons[EVENT_VOICE_AUTOWAH_DELAY] = new_Event_voice_autowah_delay;
        cons[EVENT_VOICE_SET_RESONANCE] = new_Event_voice_set_resonance;

        cons[EVENT_VOICE_SET_PANNING] = new_Event_voice_set_panning;
        cons[EVENT_VOICE_SLIDE_PANNING] = new_Event_voice_slide_panning;

        cons_initialised = true;
    }
    if (cons[type] == NULL)
    {
        return NULL; // XXX: should we consider the caller broken in this case?
    }
    return cons[type](pos);
}


