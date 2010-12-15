

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
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <math_common.h>
#include <Voice.h>
#include <Voice_state.h>
#include <Voice_params.h>
#include <xassert.h>
#include <xmemory.h>


Voice* new_Voice(void)
{
    Voice* voice = xalloc(Voice);
    if (voice == NULL)
    {
        return NULL;
    }
    voice->pool_index = 0;
    voice->id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->was_fg = true;
    voice->fg_mixed = 0;
    voice->gen = NULL;
    voice->state_size = 0;
    voice->state = NULL;

    voice->state_size = sizeof(Voice_state);
    voice->state = xalloc(Voice_state);
    if (voice->state == NULL)
    {
        del_Voice(voice);
        return NULL;
    }
    Voice_state_clear(voice->state);
    return voice;
}


bool Voice_reserve_state_space(Voice* voice, size_t state_size)
{
    assert(voice != NULL);
    if (state_size <= voice->state_size)
    {
        return true;
    }
    Voice_state* new_state = xrealloc(char, state_size, voice->state);
    if (new_state == NULL)
    {
        return false;
    }
    voice->state_size = state_size;
    voice->state = new_state;
    return true;
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
                Channel_gen_state* cgstate,
                uint32_t freq,
                double tempo)
{
    assert(voice != NULL);
    assert(gen != NULL);
    assert(params != NULL);
    assert(cgstate != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    voice->prio = VOICE_PRIO_NEW;
    voice->was_fg = true;
    voice->fg_mixed = 0;
    voice->gen = gen;
    Voice_state_init(voice->state,
                     params,
                     cgstate,
                     freq,
                     tempo);
    if (gen->init_state != NULL)
    {
        gen->init_state(gen, voice->state);
    }
    return;
}


void Voice_reset(Voice* voice)
{
    assert(voice != NULL);
    voice->id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->was_fg = true;
    voice->fg_mixed = 0;
    Voice_state_clear(voice->state);
    voice->gen = NULL;
    return;
}


void Voice_prepare(Voice* voice)
{
    assert(voice != NULL);
    voice->was_fg = voice->prio >= VOICE_PRIO_FG;
    return;
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
    Generator_mix(voice->gen, voice->state, nframes, mixed, freq, tempo);
//    fprintf(stderr, "mixed\n");
    if (!voice->state->active)
    {
//        fprintf(stderr, "not active -- setting priority %p to inactive\n",
//                (void*)&voice->prio);
        Voice_reset(voice);
//        voice->prio = VOICE_PRIO_INACTIVE;
    }
    else if (!voice->state->note_on)
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
    if (voice == NULL)
    {
        return;
    }
    xfree(voice->state);
    xfree(voice);
    return;
}


