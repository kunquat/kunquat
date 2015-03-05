

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
#include <devices/Generator.h>
#include <devices/generators/Gen_utils.h>
#include <player/Work_buffers.h>


#define RAMP_ATTACK_TIME (500.0)


void Generator_common_ramp_attack(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert((ab_count == 1) || (ab_count == 2));
    (void)gen;

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L),
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R),
    };

    const float start_ramp_attack = vstate->ramp_attack;
    const float inc = RAMP_ATTACK_TIME / freq;

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


