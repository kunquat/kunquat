

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
        Audio_buffer* voice_out_buf,
        int ab_count,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(proc != NULL);
    assert(vstate != NULL);
    assert(voice_out_buf != NULL);
    assert((ab_count == 1) || (ab_count == 2));
    (void)proc;

    kqt_frame* abufs[KQT_BUFFERS_MAX] =
    {
        Audio_buffer_get_buffer(voice_out_buf, 0),
        Audio_buffer_get_buffer(voice_out_buf, 1),
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


Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value, bool enabled)
{
    assert(cwb != NULL);
    assert(wb != NULL);

    cwb->index_mask = 0;
    cwb->def_value = def_value;
    cwb->wb_contents = &cwb->def_value;

    if (enabled)
    {
        cwb->index_mask = ~(int32_t)0;
        cwb->wb_contents = Work_buffer_get_contents(wb);
    }

    return cwb;
}


float Cond_work_buffer_get_value(const Cond_work_buffer* cwb, int32_t index)
{
    assert(cwb != NULL);
    return cwb->wb_contents[index & cwb->index_mask];
}


