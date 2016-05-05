

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


#include <player/devices/processors/Padsynth_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/processors/Proc_padsynth.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Padsynth_vstate
{
    Voice_state parent;
    double pos;
} Padsynth_vstate;


size_t Padsynth_vstate_get_size(void)
{
    return sizeof(Padsynth_vstate);
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_COUNT
};


enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int PADSYNTH_WB_FIXED_PITCH = WORK_BUFFER_IMPL_1;
static const int PADSYNTH_WB_FIXED_FORCE = WORK_BUFFER_IMPL_2;


static int32_t Padsynth_vstate_render_voice(
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

    const Device_state* dstate = &proc_state->parent;

    const Proc_padsynth* ps = (const Proc_padsynth*)dstate->device->dimpl;
    if (ps->sample == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get frequencies
    float* freqs = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
    if (freqs == NULL)
    {
        freqs = Work_buffers_get_buffer_contents_mut(wbs, PADSYNTH_WB_FIXED_PITCH);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            freqs[i] = 440;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            freqs[i] = fast_cents_to_Hz(freqs[i]);
    }

    // Get volume scales
    float* scales = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE);
    if (scales == NULL)
    {
        scales = Work_buffers_get_buffer_contents_mut(wbs, PADSYNTH_WB_FIXED_FORCE);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            scales[i] = 1;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            scales[i] = fast_dB_to_scale(scales[i]);
    }

    // Get output buffer for writing
    float* out_bufs[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_state, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_bufs);

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;

    // Render audio
    const float* sample_buf = Sample_get_buffer(ps->sample, 0);
    const int32_t length = Sample_get_len(ps->sample);
    const int32_t sample_rate = PADSYNTH_DEFAULT_AUDIO_RATE;

    const double audio_rate = dstate->audio_rate;

    const double init_pos = ps_vstate->pos;
    bool is_state_pos_updated = false;

    for (int32_t ch = 0; ch < 2; ++ch)
    {
        float* out_buf = out_bufs[ch];
        if (out_buf == NULL)
            continue;

        double pos = init_pos;
        if (ps->is_stereo_enabled && is_state_pos_updated)
        {
            pos += (length / 2);
            if (pos >= length)
                pos -= length;
        }

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float freq = freqs[i];
            const float scale = scales[i];

            const int32_t pos1 = (int32_t)pos;
            const int32_t pos2 = pos1 + 1;
            const double lerp_val = pos - floor(pos);

            const float item1 = sample_buf[pos1];
            const float item2 = sample_buf[pos2];
            const double value = lerp(item1, item2, lerp_val);

            out_buf[i] = value * scale;

            pos += (freq / 440.0) * (sample_rate / audio_rate);

            while (pos >= length)
                pos -= length;
        }

        if (!ps->is_stereo_enabled || !is_state_pos_updated)
        {
            ps_vstate->pos = pos;
            is_state_pos_updated = true;
        }
    }


    if (ps->is_ramp_attack_enabled)
        Proc_ramp_attack(vstate, 2, out_bufs, buf_start, buf_stop, dstate->audio_rate);

    return buf_stop;
}


void Padsynth_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Padsynth_vstate_render_voice;

    const Proc_padsynth* ps = (const Proc_padsynth*)proc_state->parent.device->dimpl;
    const int32_t sample_length = Sample_get_len(ps->sample);

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;
    ps_vstate->pos = Random_get_index(vstate->rand_p, sample_length);

    return;
}


