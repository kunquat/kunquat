

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
#include <stdio.h>
#include <stdint.h>

#include <Voice.h>
#include <Event_voice.h>
#include <Channel_state.h>

#include <xmemory.h>


Voice* new_Voice(uint8_t events)
{
    assert(events > 0);
    Voice* voice = xalloc(Voice);
    if (voice == NULL)
    {
        return NULL;
    }
    voice->events = new_Event_queue(events);
    if (voice->events == NULL)
    {
        xfree(voice);
        return NULL;
    }
    voice->pool_index = 0;
    voice->id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->gen = NULL;
    Voice_state_clear(&voice->state.generic);
    return voice;
}


int Voice_cmp(Voice* v1, Voice* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);
    return v1->prio - v2->prio;
}


uint64_t Voice_id(Voice* voice)
{
    assert(voice != NULL);
    return voice->id;
}


void Voice_init(Voice* voice,
                Generator* gen,
                const Channel_state* cur_ch_state,
                Channel_state* new_ch_state,
                uint32_t freq,
                double tempo)
{
    assert(voice != NULL);
    assert(gen != NULL);
    assert(cur_ch_state != NULL);
    assert(new_ch_state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    voice->prio = VOICE_PRIO_NEW;
    voice->gen = gen;
    Voice_state_init(&voice->state.generic,
                     cur_ch_state,
                     new_ch_state,
                     freq,
                     tempo);
    if (gen->init_state != NULL)
    {
        gen->init_state(gen, &voice->state.generic);
    }
    Event_queue_clear(voice->events);
    return;
}


void Voice_reset(Voice* voice)
{
    assert(voice != NULL);
    voice->prio = VOICE_PRIO_INACTIVE;
    Event_queue_clear(voice->events);
    Voice_state_clear(&voice->state.generic);
    voice->gen = NULL;
    return;
}


bool Voice_add_event(Voice* voice, Event* event, uint32_t pos)
{
    assert(voice != NULL);
    assert(event != NULL);
    return Event_queue_ins(voice->events, event, pos);
}


void Voice_mix(Voice* voice,
               uint32_t nframes,
               uint32_t offset,
               uint32_t freq,
               double tempo)
{
    assert(voice != NULL);
    assert(voice->gen != NULL);
    assert(freq > 0);
    if (voice->prio == VOICE_PRIO_INACTIVE)
    {
        return;
    }
    uint32_t mixed = offset;
    Event* next = NULL;
    uint32_t next_pos = UINT32_MAX;
    bool event_found = Event_queue_peek(voice->events, 0, &next, &next_pos);
    if (event_found && Event_get_type(next) == EVENT_VOICE_NOTE_ON)
    {
        mixed = next_pos;
    }
    uint32_t mix_until = next_pos;
    while (mixed < nframes)
    {
        if (mix_until > nframes)
        {
            mix_until = nframes;
        }
        if (voice->prio < VOICE_PRIO_NEW)
        {
            Generator_mix(voice->gen, &voice->state.generic, mix_until, mixed, freq, tempo);
        }
        else
        {
            assert(event_found);
            assert(next != NULL);
            assert(Event_get_type(next) == EVENT_VOICE_NOTE_ON);
            if (next_pos <= mix_until)
            {
                voice->prio = VOICE_PRIO_NEW - 1;
            }
        }
        if (event_found && next_pos <= mix_until)
        {
            Event_queue_get(voice->events, &next, &next_pos);
            assert(next != NULL);
            Event_voice_process((Event_voice*)next, voice);
        }
        mixed = mix_until;
        next_pos = UINT32_MAX;
        event_found = Event_queue_peek(voice->events, 0, &next, &next_pos);
        mix_until = next_pos;
    }
    while (Event_queue_peek(voice->events, 0, &next, &next_pos) && next_pos <= nframes)
    {
        Event_queue_get(voice->events, &next, &next_pos);
        assert(next != NULL);
        if (Event_get_type(next) == EVENT_VOICE_NOTE_ON && voice->prio == VOICE_PRIO_NEW)
        {
            voice->prio = VOICE_PRIO_NEW - 1;
        }
        Event_voice_process((Event_voice*)next, voice);
    }
#if 0
    bool event_found = Event_queue_get(voice->events, &next, &mix_until);
    if (event_found && Event_get_type(next) == EVENT_VOICE_NOTE_ON)
    {
        mixed = mix_until;
    }
    while (mixed < nframes || event_found)
    {
        if (mix_until > nframes)
        {
            mix_until = nframes;
        }
        if (voice->prio < VOICE_PRIO_NEW)
        {
            Generator_mix(voice->gen, &voice->state.generic, mix_until, mixed, freq, tempo);
        }
        else
        {
            voice->prio = VOICE_PRIO_NEW - 1;
        }
        if (event_found)
        {
            assert(EVENT_IS_VOICE(Event_get_type(next)));
            Event_voice_process((Event_voice*)next, voice);
        }
        mixed = mix_until;
        mix_until = nframes;
        event_found = Event_queue_get(voice->events, &next, &mix_until);
    }
#endif
    if (!voice->state.generic.active)
    {
        voice->prio = VOICE_PRIO_INACTIVE;
    }
    else if (!voice->state.generic.note_on)
    {
        voice->prio = VOICE_PRIO_BG;
    }
//    Event_queue_clear(voice->events);
    return;
}


void del_Voice(Voice* voice)
{
    assert(voice != NULL);
    del_Event_queue(voice->events);
    xfree(voice);
    return;
}


