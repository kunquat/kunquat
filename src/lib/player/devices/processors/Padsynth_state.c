

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
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Padsynth_vstate
{
    Voice_state parent;

    double init_pitch;
    double pos;
} Padsynth_vstate;


int32_t Padsynth_vstate_get_size(void)
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

    if (buf_start == buf_stop)
        return buf_start;

    const Device_state* dstate = &proc_state->parent;

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;

    const Proc_padsynth* ps = (const Proc_padsynth*)dstate->device->dimpl;
    if (ps->sample_map == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get frequencies
    Work_buffer* freqs_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PITCH);
    Work_buffer* pitches_wb = freqs_wb;

    if (isnan(ps_vstate->init_pitch))
        ps_vstate->init_pitch =
            (pitches_wb != NULL) ? Work_buffer_get_contents(pitches_wb)[buf_start] : 0;

    if (freqs_wb == NULL)
        freqs_wb = Work_buffers_get_buffer_mut(wbs, PADSYNTH_WB_FIXED_PITCH);
    Proc_fill_freq_buffer(freqs_wb, pitches_wb, buf_start, buf_stop);
    const float* freqs = Work_buffer_get_contents(freqs_wb);

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE);
    Work_buffer* dBs_wb = scales_wb;
    if (scales_wb == NULL)
        scales_wb = Work_buffers_get_buffer_mut(wbs, PADSYNTH_WB_FIXED_FORCE);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, buf_start, buf_stop);
    const float* scales = Work_buffer_get_contents(scales_wb);

    // Get output buffer for writing
    float* out_bufs[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_bufs);

    // Choose our sample
    const Padsynth_sample_entry* entry =
        Padsynth_sample_map_get_entry(ps->sample_map, ps_vstate->init_pitch);
    if (entry == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Render audio
    const float* sample_buf = Sample_get_buffer(entry->sample, 0);
    const int32_t length = Padsynth_sample_map_get_sample_length(ps->sample_map);
    const int32_t sample_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    const double sample_freq = cents_to_Hz(entry->center_pitch);

    const double audio_rate = dstate->audio_rate;

    const double init_pos = fmod(ps_vstate->pos, length); // the length may have changed
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
            const float value = (float)lerp(item1, item2, lerp_val);

            out_buf[i] = value * scale;

            pos += (freq / sample_freq) * (sample_rate / audio_rate);

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
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Padsynth_vstate_render_voice;

    const Proc_padsynth* ps = (const Proc_padsynth*)proc_state->parent.device->dimpl;
    const int32_t sample_length = Padsynth_sample_map_get_sample_length(ps->sample_map);

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;
    ps_vstate->init_pitch = NAN;
    ps_vstate->pos = Random_get_index(vstate->rand_p, sample_length);

    return;
}


