

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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <Voice.h>
#include <Voice_params.h>

#include <xmemory.h>


Voice* new_Voice(void)
{
//    assert(events > 0);
    Voice* voice = xalloc(Voice);
    if (voice == NULL)
    {
        return NULL;
    }
#if 0
    voice->events = new_Event_queue(events);
    if (voice->events == NULL)
    {
        xfree(voice);
        return NULL;
    }
#endif
    voice->pool_index = 0;
    voice->id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->was_fg = true;
    voice->fg_mixed = 0;
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
                Voice_params* params,
                uint32_t freq,
                double tempo)
{
    assert(voice != NULL);
    assert(gen != NULL);
    assert(params != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    voice->prio = VOICE_PRIO_NEW;
    voice->was_fg = true;
    voice->fg_mixed = 0;
    voice->gen = gen;
    Voice_state_init(&voice->state.generic,
                     params,
                     freq,
                     tempo);
    if (gen->init_state != NULL)
    {
        gen->init_state(gen, &voice->state.generic);
    }
//    Event_queue_clear(voice->events);
    return;
}


void Voice_reset(Voice* voice)
{
    assert(voice != NULL);
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->was_fg = true;
    voice->fg_mixed = 0;
//    Event_queue_clear(voice->events);
    Voice_state_clear(&voice->state.generic);
    voice->gen = NULL;
    return;
}


#if 0
bool Voice_add_event(Voice* voice, Event* event, uint32_t pos)
{
    assert(voice != NULL);
    assert(event != NULL);
    return Event_queue_ins(voice->events, event, pos);
}
#endif


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
    bool initially_fg = voice->prio >= VOICE_PRIO_FG;
    uint32_t mixed = offset;
    if (voice->prio <= VOICE_PRIO_BG)
    {
        if (voice->was_fg)
        {
//            fprintf(stderr, "mixed: %" PRIu32 ", fg_mixed: %" PRIu32 "\n", mixed, voice->fg_mixed);
            assert(mixed <= voice->fg_mixed);
//            fprintf(stderr, "setting voice->fg_mixed: %" PRIu32 "\n", voice->fg_mixed);
            mixed = voice->fg_mixed;
            voice->was_fg = false;
            voice->fg_mixed = 0;
        }
    }
//    fprintf(stderr, "mix %p from %" PRIu32 " to %" PRIu32 "\n", (void*)voice, mixed, nframes);
    Generator_mix(voice->gen, &voice->state.generic, nframes, mixed, freq, tempo);
    if (!voice->state.generic.active)
    {
        voice->prio = VOICE_PRIO_INACTIVE;
    }
    else if (!voice->state.generic.note_on)
    {
        voice->prio = VOICE_PRIO_BG;
    }
    if (initially_fg)
    {
        voice->fg_mixed = nframes;
    }
    return;
}


void del_Voice(Voice* voice)
{
    assert(voice != NULL);
//    del_Event_queue(voice->events);
    xfree(voice);
    return;
}


