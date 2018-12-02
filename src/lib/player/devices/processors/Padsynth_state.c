

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
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


void Padsynth_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);
    rassert(groups != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


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


int32_t Padsynth_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(tempo > 0);

    const Device_state* dstate = &proc_state->parent;

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;

    const Proc_padsynth* ps = (const Proc_padsynth*)dstate->device->dimpl;
    if (ps->sample_map == NULL)
    {
        vstate->active = false;
        return 0;
    }

    // Get frequencies
    Work_buffer* freqs_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PITCH, NULL);
    Work_buffer* pitches_wb = freqs_wb;

    if (isnan(ps_vstate->init_pitch))
        ps_vstate->init_pitch = (pitches_wb != NULL)
            ? Work_buffer_get_contents(pitches_wb, 0)[0] : 0;

    if (freqs_wb == NULL)
        freqs_wb = Work_buffers_get_buffer_mut(wbs, PADSYNTH_WB_FIXED_PITCH, 1);
    Proc_fill_freq_buffer(freqs_wb, pitches_wb, 0, frame_count);
    const float* freqs = Work_buffer_get_contents(freqs_wb, 0);

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    Work_buffer* dBs_wb = scales_wb;
    if ((dBs_wb != NULL) &&
            Work_buffer_is_final(dBs_wb, 0) &&
            (Work_buffer_get_const_start(dBs_wb, 0) == 0) &&
            (Work_buffer_get_contents(dBs_wb, 0)[0] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        vstate->active = false;
        return 0;
    }

    if (scales_wb == NULL)
        scales_wb = Work_buffers_get_buffer_mut(wbs, PADSYNTH_WB_FIXED_FORCE, 1);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, frame_count);
    const float* scales = Work_buffer_get_contents(scales_wb, 0);

    // Get output buffer for writing
    Work_buffer* out_wb = Proc_get_voice_output_2ch(proc_ts, PORT_OUT_AUDIO_L);
    rassert(out_wb != NULL);

    // Choose our sample
    const Padsynth_sample_entry* entry =
        Padsynth_sample_map_get_entry(ps->sample_map, ps_vstate->init_pitch);
    if (entry == NULL)
    {
        vstate->active = false;
        return 0;
    }

    // Render audio
    const float* sample_buf = Sample_get_buffer(entry->sample, 0);
    const int32_t length = Padsynth_sample_map_get_sample_length(ps->sample_map);
    const int32_t sample_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    const double sample_freq = cents_to_Hz(entry->centre_pitch);

    const double rel_sample_rate = sample_rate / dstate->audio_rate;

    const double init_pos = fmod(ps_vstate->pos, length); // the length may have changed
    //bool is_state_pos_updated = false;

    double pos[2] = { init_pos, init_pos };
    if (ps->is_stereo_enabled)
    {
        pos[1] += (length / 2);
        if (pos[1] >= length)
            pos[1] -= length;
    }

    float* out = Work_buffer_get_contents_mut(out_wb, 0);

    for (int32_t i = 0; i < frame_count; ++i)
    {
        const float freq = freqs[i];
        const float scale = scales[i];

        const double rel_freq = freq / sample_freq;

        for (int ch = 0; ch < 2; ++ch)
        {
            double pos_ch = pos[ch];

            const int32_t pos1 = (int32_t)pos_ch;
            const int32_t pos2 = pos1 + 1;
            const double lerp_val = pos_ch - floor(pos_ch);

            const float item1 = sample_buf[pos1];
            const float item2 = sample_buf[pos2];
            const float value = (float)lerp(item1, item2, lerp_val);

            *out++ = value * scale;

            pos_ch += rel_freq * rel_sample_rate;
            if (pos_ch >= length)
            {
                pos_ch -= length;

                if (pos_ch >= length)
                    pos_ch = pos[ch]; // the pitch is insane, so skip updating
            }

            pos[ch] = pos_ch;
        }
    }

    ps_vstate->pos = pos[0];

#if 0
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

        for (int32_t i = 0; i < frame_count; ++i)
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
#endif

    if (ps->is_ramp_attack_enabled)
        Proc_ramp_attack(vstate, out_wb, frame_count, dstate->audio_rate);

    return frame_count;
}


void Padsynth_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    const Proc_padsynth* ps = (const Proc_padsynth*)proc_state->parent.device->dimpl;
    const int32_t sample_length = Padsynth_sample_map_get_sample_length(ps->sample_map);

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;
    ps_vstate->init_pitch = NAN;
    ps_vstate->pos = Random_get_index(vstate->rand_p, sample_length);

    return;
}


