

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
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


bool Voice_reserve_state_space(Voice* voice, int32_t state_size)
{
    assert(voice != NULL);
    assert(state_size >= 0);

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

    return (int)v1->prio - (int)v2->prio;
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
        uint64_t seed)
{
    assert(voice != NULL);
    assert(proc != NULL);
    assert(proc_state != NULL);

    voice->prio = VOICE_PRIO_NEW;
    voice->proc = proc;
    voice->group_id = group_id;
    Random_set_seed(voice->rand_p, seed);
    Random_set_seed(voice->rand_s, seed);

    Voice_state_init(voice->state, voice->rand_p, voice->rand_s);

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


int32_t Voice_render(
        Voice* voice,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(voice != NULL);
    assert(voice->proc != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= buf_start);

    if (voice->prio == VOICE_PRIO_INACTIVE)
        return buf_stop;

    Proc_state* pstate = (Proc_state*)Device_states_get_state(
            dstates, Device_get_id((const Device*)voice->proc));
    const Au_state* au_state = (const Au_state*)Device_states_get_state(
            dstates, voice->proc->au_params->device_id);

    const int32_t process_stop = Voice_state_render_voice(
            voice->state, pstate, au_state, wbs, buf_start, buf_stop, tempo);
    ignore(process_stop); // TODO: this should probably be release_stop

    voice->updated = true;

    if (!voice->state->active)
    {
        Voice_reset(voice);
        return buf_start;
    }
    else if (!voice->state->note_on)
    {
        voice->prio = VOICE_PRIO_BG;
        if (voice->state->has_release_data)
            return voice->state->release_stop;

        return buf_start;
    }

    return buf_stop;
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


