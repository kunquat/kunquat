

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Force_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_force.h>

#include <stdlib.h>


typedef struct Force_vstate
{
    Voice_state parent;
} Force_vstate;


size_t Force_vstate_get_size(void)
{
    return sizeof(Force_vstate);
}


static int32_t Force_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    //Force_vstate* fvstate = (Force_vstate*)vstate;

    // Get output
    float* out_buf =
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Fill in the force values
    const float* actual_forces =
        Work_buffers_get_buffer_contents(wbs, WORK_BUFFER_ACTUAL_FORCES);
    for (int32_t i = buf_start; i < buf_stop; ++i)
        out_buf[i] = actual_forces[i];

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Force_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Force_vstate_render_voice;

    return;
}


