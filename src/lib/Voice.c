

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <memory.h>
#include <Random.h>
#include <Voice.h>
#include <Voice_state.h>
#include <Voice_params.h>
#include <xassert.h>


Voice* new_Voice(void)
{
    Voice* voice = memory_alloc_item(Voice);
    if (voice == NULL)
    {
        return NULL;
    }
    voice->id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->gen = NULL;
    voice->state_size = 0;
    voice->state = NULL;

    voice->state_size = sizeof(Voice_state);
    voice->state = memory_alloc_item(Voice_state);
    voice->rand_p = new_Random();
    voice->rand_s = new_Random();
    if (voice->state == NULL || voice->rand_p == NULL ||
            voice->rand_s == NULL)
    {
        del_Voice(voice);
        return NULL;
    }
    Random_set_context(voice->rand_p, "vp");
    Random_set_context(voice->rand_s, "vs");
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
    Voice_state* new_state = memory_realloc_items(char, state_size, voice->state);
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
                uint64_t seed,
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
    voice->gen = gen;
    Random_set_seed(voice->rand_p, seed);
    Random_set_seed(voice->rand_s, seed);
    Voice_state_init(voice->state,
                     params,
                     cgstate,
                     voice->rand_p,
                     voice->rand_s,
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
    Voice_state_clear(voice->state);
    voice->gen = NULL;
    Random_reset(voice->rand_p);
    Random_reset(voice->rand_s);
    return;
}


void Voice_prepare(Voice* voice)
{
    assert(voice != NULL);
    (void)voice;
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
    uint32_t mixed = offset;
    Generator_mix(voice->gen, voice->state, nframes, mixed, freq, tempo);
    if (!voice->state->active)
    {
        Voice_reset(voice);
    }
    else if (!voice->state->note_on)
    {
        voice->prio = VOICE_PRIO_BG;
    }
    return;
}


double Voice_get_actual_force(Voice* voice)
{
    assert(voice != NULL);
    assert(voice->state != NULL);
    assert(voice->state->active);
    assert(voice->state->actual_force > 0);
    return log2(voice->state->actual_force) * 6;
}


void del_Voice(Voice* voice)
{
    if (voice == NULL)
    {
        return;
    }
    del_Random(voice->rand_p);
    del_Random(voice->rand_s);
    memory_free(voice->state);
    memory_free(voice);
    return;
}


