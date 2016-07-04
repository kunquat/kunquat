

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Noise_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_noise.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Filter.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define NOISE_MAX 8


typedef struct Noise_pstate
{
    Proc_state parent;
    int order;
} Noise_pstate;


Device_state* new_Noise_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Noise_pstate* noise_state = memory_alloc_item(Noise_pstate);
    if (noise_state == NULL)
        return NULL;

    if (!Proc_state_init(&noise_state->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(noise_state);
        return NULL;
    }

    noise_state->order = 0;

    return &noise_state->parent.parent;
}


typedef struct Noise_vstate
{
    Voice_state parent;
    double buf[2][NOISE_MAX];
} Noise_vstate;


int32_t Noise_vstate_get_size(void)
{
    return sizeof(Noise_vstate);
}


static int32_t Noise_vstate_render_voice(
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
    assert(tempo > 0);

//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);

    Noise_pstate* noise_state = (Noise_pstate*)proc_state;
    Noise_vstate* noise_vstate = (Noise_vstate*)vstate;

    // Get output buffer for writing
    float* out_buffers[] =
    {
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0),
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 1),
    };

    for (int ch = 0; ch < 2; ++ch)
    {
        if (out_buffers[ch] == NULL)
            continue;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            double val = 0;

            if (noise_state->order < 0)
            {
                val = dc_pole_filter(
                        -noise_state->order,
                        noise_vstate->buf[ch],
                        Random_get_float_signal(vstate->rand_s));
            }
            else
            {
                val = dc_zero_filter(
                        noise_state->order,
                        noise_vstate->buf[ch],
                        Random_get_float_signal(vstate->rand_s));
            }

            out_buffers[ch][i] = (float)val;
        }
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;
    Proc_ramp_attack(vstate, 2, out_buffers, buf_start, buf_stop, audio_rate);

//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return buf_stop;
}


void Noise_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Noise_vstate_render_voice;

    Noise_vstate* noise_vstate = (Noise_vstate*)vstate;
    memset(noise_vstate->buf[0], 0, NOISE_MAX * sizeof(double));
    memset(noise_vstate->buf[1], 0, NOISE_MAX * sizeof(double));

    return;
}


