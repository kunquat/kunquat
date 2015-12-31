

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Voice_state.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


Voice* new_Voice(void)
{
    Voice* voice = memory_alloc_item(Voice);
    if (voice == NULL)
        return NULL;

    voice->id = 0;
    voice->group_id = 0;
    voice->updated = false;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->proc = NULL;
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
        return true;

    Voice_state* new_state = memory_realloc_items(char, state_size, voice->state);
    if (new_state == NULL)
        return false;

    voice->state_size = state_size;
    voice->state = new_state;

    return true;
}


int Voice_cmp(const Voice* v1, const Voice* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);

    return v1->prio - v2->prio;
}


uint64_t Voice_id(const Voice* voice)
{
    assert(voice != NULL);
    return voice->id;
}


uint64_t Voice_get_group_id(const Voice* voice)
{
    assert(voice != NULL);
    return voice->group_id;
}


const Processor* Voice_get_proc(const Voice* voice)
{
    assert(voice != NULL);
    return voice->proc;
}


void Voice_init(
        Voice* voice,
        const Processor* proc,
        uint64_t group_id,
        const Proc_state* proc_state,
        uint64_t seed,
        uint32_t freq,
        double tempo)
{
    assert(voice != NULL);
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    voice->prio = VOICE_PRIO_NEW;
    voice->proc = proc;
    voice->group_id = group_id;
    Random_set_seed(voice->rand_p, seed);
    Random_set_seed(voice->rand_s, seed);

    Voice_state_init(voice->state, voice->rand_p, voice->rand_s, freq, tempo);

    const Device_impl* dimpl = Device_get_impl((const Device*)proc);
    if ((dimpl != NULL) && (dimpl->init_vstate != NULL))
        dimpl->init_vstate(voice->state, proc_state);

    return;
}


void Voice_reset(Voice* voice)
{
    assert(voice != NULL);

    voice->id = 0;
    voice->group_id = 0;
    voice->prio = VOICE_PRIO_INACTIVE;
    Voice_state_clear(voice->state);
    voice->proc = NULL;
    Random_reset(voice->rand_p);
    Random_reset(voice->rand_s);

    return;
}


void Voice_mix(
        Voice* voice,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(voice != NULL);
    assert(voice->proc != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(freq > 0);

    if (voice->prio == VOICE_PRIO_INACTIVE)
        return;

    //Processor_process_vstate(
    //        voice->proc, states, voice->state, wbs, offset, nframes, freq, tempo);

    Proc_state* pstate = (Proc_state*)Device_states_get_state(
            dstates, Device_get_id((const Device*)voice->proc));
    const Au_state* au_state = (const Au_state*)Device_states_get_state(
            dstates, voice->proc->au_params->device_id);

    Voice_state_render_voice(
            voice->state, pstate, au_state, wbs, offset, nframes, tempo);

    voice->updated = true;

    if (!voice->state->active)
        Voice_reset(voice);
    else if (!voice->state->note_on)
        voice->prio = VOICE_PRIO_BG;

    return;
}


double Voice_get_actual_force(const Voice* voice)
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
        return;

    del_Random(voice->rand_p);
    del_Random(voice->rand_s);
    memory_free(voice->state);
    memory_free(voice);

    return;
}


