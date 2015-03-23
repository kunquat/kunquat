

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


#include <stdlib.h>

#include <debug/assert.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <memory.h>
#include <player/Proc_state.h>
#include <player/Work_buffers.h>


#define RAMP_ATTACK_TIME (500.0)


Device_state* new_Proc_state_default(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* pstate = memory_alloc_item(Proc_state);
    if ((pstate == NULL) ||
            !Proc_state_init(pstate, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(&pstate->parent);
        return NULL;
    }

    return &pstate->parent;
}


void Proc_ramp_attack(
        const Processor* proc,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(proc != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert((ab_count == 1) || (ab_count == 2));
    (void)proc;

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L),
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R),
    };

    const float start_ramp_attack = vstate->ramp_attack;
    const float inc = RAMP_ATTACK_TIME / audio_rate;

    for (int ch = 0; ch < ab_count; ++ch)
    {
        float ramp_attack = start_ramp_attack;

        for (int32_t i = buf_start; (i < buf_stop) && (ramp_attack < 1); ++i)
        {
            abufs[ch][i] *= ramp_attack;
            ramp_attack += inc;
        }

        vstate->ramp_attack = ramp_attack;
    }

    return;
}


