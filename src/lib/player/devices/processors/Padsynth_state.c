

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
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
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PITCH);
    Work_buffer* pitches_wb = freqs_wb;

    const bool needs_init = isnan(ps_vstate->init_pitch);

    if (needs_init)
        ps_vstate->init_pitch =
            ((pitches_wb != NULL) && Work_buffer_is_valid(pitches_wb, 0))
            ? Work_buffer_get_contents(pitches_wb, 0)[0] : 0;

    if (freqs_wb == NULL)
        freqs_wb = Work_buffers_get_buffer_mut(wbs, PADSYNTH_WB_FIXED_PITCH, 1);
    Proc_fill_freq_buffer(freqs_wb, pitches_wb, 0, frame_count);
    const float* freqs = Work_buffer_get_contents(freqs_wb, 0);

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE);
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
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);
    if ((out_wbs[0] == NULL) && (out_wbs[1] == NULL))
    {
        vstate->active = false;
        return 0;
    }

    // Choose our sample
    const Padsynth_sample_entry* entry =
        Padsynth_sample_map_get_entry(ps->sample_map, ps_vstate->init_pitch);
    if (entry == NULL)
    {
        vstate->active = false;
        return 0;
    }

    if (needs_init)
    {
        // Set initial playback start position
        const int32_t sample_length =
            Padsynth_sample_map_get_sample_length(ps->sample_map);
        ps_vstate->pos = ps->start_pos * sample_length;

        if (ps->is_start_pos_var_enabled)
        {
            if ((ps->start_pos_var == 1.0) && !ps->round_start_pos_var_to_period)
            {
                // Preserve backwards compatibility
                ps_vstate->pos = Random_get_index(vstate->rand_p, sample_length);
            }
            else
            {
                const double shift_norm =
                    (Random_get_float_lb(vstate->rand_p) - 0.5) * ps->start_pos_var;
                double shift = shift_norm * sample_length;
                if (ps->round_start_pos_var_to_period)
                {
                    const double entry_Hz = cents_to_Hz(entry->centre_pitch);
                    const double cycle_length = PADSYNTH_DEFAULT_AUDIO_RATE / entry_Hz;
                    shift = round(shift / cycle_length) * cycle_length;
                }

                ps_vstate->pos += shift + sample_length;
            }
        }

        if (ps_vstate->pos >= sample_length)
            ps_vstate->pos = fmod(ps_vstate->pos, sample_length);

        rassert(ps_vstate->pos >= 0);
        rassert(ps_vstate->pos < sample_length);
    }

    // Render audio
    const float* sample_buf = Sample_get_buffer(entry->sample, 0);
    const int32_t length = Padsynth_sample_map_get_sample_length(ps->sample_map);
    const int32_t sample_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    const double sample_freq = cents_to_Hz(entry->centre_pitch);

    const double rel_sample_rate = sample_rate / (double)dstate->audio_rate;

    const double init_pos = fmod(ps_vstate->pos, length); // the length may have changed
    double state_pos[2] = { init_pos, init_pos };
    if (ps->is_stereo_enabled)
    {
        state_pos[1] += (length / 2);
        if (state_pos[1] >= length)
            state_pos[1] -= length;
    }

    for (int ch = 0; ch < 2; ++ch)
    {
        if (out_wbs[ch] == NULL)
            continue;

        double pos = state_pos[ch];

        float* out = Work_buffer_get_contents_mut(out_wbs[ch], 0);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float freq = freqs[i];
            const float scale = scales[i];

            const double rel_freq = freq / sample_freq;

            const int32_t pos1 = (int32_t)pos;
            const int32_t pos2 = pos1 + 1;
            const double lerp_val = pos - floor(pos);

            const float item1 = sample_buf[pos1];
            const float item2 = sample_buf[pos2];
            const float value = (float)lerp(item1, item2, lerp_val);

            *out++ = value * scale;

            pos += rel_freq * rel_sample_rate;
            if (pos >= length)
            {
                pos -= length;

                if (pos >= length)
                    pos = init_pos; // the pitch is insane, so skip updating
            }
        }

        state_pos[ch] = pos;
    }

    if (out_wbs[0] != NULL)
    {
        ps_vstate->pos = state_pos[0];
    }
    else
    {
        ps_vstate->pos = state_pos[1];
        if (ps->is_stereo_enabled)
        {
            ps_vstate->pos -= (length / 2);
            if (ps_vstate->pos < 0)
                ps_vstate->pos += length;
        }
    }

    if (ps->is_ramp_attack_enabled)
        Proc_ramp_attack(vstate, 2, out_wbs, frame_count, dstate->audio_rate);

    return frame_count;
}


void Padsynth_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Padsynth_vstate* ps_vstate = (Padsynth_vstate*)vstate;
    ps_vstate->init_pitch = NAN;

    return;
}


