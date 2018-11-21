

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
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
#include <mathnum/Random.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Filter.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

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
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

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
    Random rands[2];
    double buf[2][NOISE_MAX];
} Noise_vstate;


int32_t Noise_vstate_get_size(void)
{
    return sizeof(Noise_vstate);
}


enum
{
    PORT_IN_FORCE = 0,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int NOISE_WB_FIXED_FORCE = WORK_BUFFER_IMPL_1;


int32_t Noise_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(tempo > 0);

//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    Work_buffer* dBs_wb = scales_wb;
    if ((dBs_wb != NULL) &&
            Work_buffer_is_valid(dBs_wb) &&
            Work_buffer_is_final(dBs_wb, 0) &&
            (Work_buffer_get_const_start(dBs_wb, 0) <= buf_start) &&
            (Work_buffer_get_contents(dBs_wb, 0)[buf_start] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        vstate->active = false;
        return buf_start;
    }

    if ((scales_wb == NULL) || !Work_buffer_is_valid(scales_wb))
        scales_wb = Work_buffers_get_buffer_mut(wbs, NOISE_WB_FIXED_FORCE, 1);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, buf_start, buf_stop);
    const float* scales = Work_buffer_get_contents(scales_wb, 0);

    Noise_pstate* noise_state = (Noise_pstate*)proc_state;
    Noise_vstate* noise_vstate = (Noise_vstate*)vstate;

    // Get output buffer for writing
    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_buffers);

    for (int ch = 0; ch < 2; ++ch)
    {
        float* out_buffer = out_buffers[ch];
        if (out_buffer == NULL)
            continue;

        if (noise_state->order >= 0)
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const double val = dc_zero_filter(
                        noise_state->order,
                        noise_vstate->buf[ch],
                        Random_get_float_signal(&noise_vstate->rands[ch]));
                out_buffer[i] = (float)val * scales[i];
            }
        }
        else
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const double val = dc_pole_filter(
                        -noise_state->order,
                        noise_vstate->buf[ch],
                        Random_get_float_signal(&noise_vstate->rands[ch]));
                out_buffer[i] = (float)val * scales[i];
            }
        }
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;
    Proc_ramp_attack(vstate, 2, out_buffers, buf_start, buf_stop, audio_rate);

//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return buf_stop;
}


void Noise_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Noise_vstate* noise_vstate = (Noise_vstate*)vstate;

    for (int i = 0; i < 2; ++i)
    {
        Random* rand = &noise_vstate->rands[i];
        Random_init(rand, "noise");
        Random_set_seed(rand, Random_get_uint64(vstate->rand_p));
    }

    memset(noise_vstate->buf[0], 0, NOISE_MAX * sizeof(double));
    memset(noise_vstate->buf[1], 0, NOISE_MAX * sizeof(double));

    return;
}


