

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


#include <player/devices/processors/Ks_state.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define DELAY_BUF_LEN_MAX 4096


typedef struct Ks_vstate
{
    Voice_state parent;

    double init_pitch;
    double buf_pos;
    float delay_buf[DELAY_BUF_LEN_MAX];
} Ks_vstate;


int32_t Ks_vstate_get_size(void)
{
    return sizeof(Ks_vstate);
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_COUNT
};


enum
{
    PORT_OUT_AUDIO = 0,
    PORT_OUT_COUNT
};


static const int KS_WB_FIXED_PITCH = WORK_BUFFER_IMPL_1;
static const int KS_WB_FIXED_FORCE = WORK_BUFFER_IMPL_2;


static int32_t Ks_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);
    rassert(tempo > 0);

    if (buf_start == buf_stop)
        return buf_start;

    const Device_state* dstate = &proc_state->parent;

    Ks_vstate* ks_vstate = (Ks_vstate*)vstate;

    // Get frequencies
    Work_buffer* freqs_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
    const Work_buffer* pitches_wb = freqs_wb;

    const bool need_init = isnan(ks_vstate->init_pitch);
    if (need_init)
        ks_vstate->init_pitch =
            (pitches_wb != NULL) ? Work_buffer_get_contents(pitches_wb)[buf_start] : 0;

    if (freqs_wb == NULL)
        freqs_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_PITCH);
    Proc_fill_freq_buffer(freqs_wb, pitches_wb, buf_start, buf_stop);
    //const float* freqs = Work_buffer_get_contents(freqs_wb);

    // Get volume scales
    Work_buffer* scales_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE);
    const Work_buffer* dBs_wb = scales_wb;
    if (scales_wb == NULL)
        scales_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_FORCE);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, buf_start, buf_stop);
    const float* scales = Work_buffer_get_contents(scales_wb);

    // Get output buffer for writing
    float* out_buf = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO);
    if (out_buf == NULL)
        return buf_start;

    // Get delay buffer length
    const int32_t audio_rate = dstate->audio_rate;
    const double init_freq = cents_to_Hz(ks_vstate->init_pitch);
    const double period_length = audio_rate / init_freq;
    const double used_buf_length = clamp(period_length, 2, DELAY_BUF_LEN_MAX);
    const int32_t used_buf_frames_whole = (int32_t)floor(used_buf_length);
    const int32_t used_buf_frames = (int32_t)ceil(used_buf_length);
    rassert(used_buf_frames <= DELAY_BUF_LEN_MAX);

    if (need_init)
    {
        for (int32_t i = 0; i < used_buf_frames; ++i)
            ks_vstate->delay_buf[i] = (float)Random_get_float_signal(vstate->rand_s);
    }

    double pos = ks_vstate->buf_pos;
    float* delay_buf = ks_vstate->delay_buf;

    for (int32_t i = buf_start; i < buf_stop;)
    {
        const double lerp_val = pos - floor(pos);

        // The easy part
        for (; i < buf_stop && pos < used_buf_frames - 1; ++i, pos += 1)
        {
            const float scale = scales[i];

            const int32_t pos1 = (int32_t)pos;
            const int32_t pos2 = pos1 + 1;

            const float item1 = delay_buf[pos1];
            const float item2 = delay_buf[pos2];
            const float value = (float)lerp(item1, item2, lerp_val);

            out_buf[i] = value * scale;

            delay_buf[pos1] = (item1 + item2) * 0.5f;
        }

        // Handle wrap-around
        if (i < buf_stop)
        {
            const float scale = scales[i];

            float value = 0.0f;

            if (used_buf_frames_whole < used_buf_frames)
            {
                const double last_rem = used_buf_length - used_buf_frames_whole;
                if (lerp_val < last_rem)
                {
                    const double last_lerp_val = lerp_val / last_rem;

                    const int32_t pos1 = (int32_t)pos;
                    const int32_t pos2 = 0;

                    const float item1 = delay_buf[pos1];
                    const float item2 = delay_buf[pos2];
                    value = (float)lerp(item1, item2, last_lerp_val);

                    delay_buf[pos1] = (item1 + item2) * 0.5f;
                }
                else
                {
                    const double new_lerp_val = lerp_val - last_rem;

                    const int32_t pos0 = (int32_t)pos;
                    const int32_t pos1 = 0;
                    const int32_t pos2 = 1;

                    const float item0 = delay_buf[pos0];
                    const float item1 = delay_buf[pos1];
                    const float item2 = delay_buf[pos2];
                    value = (float)lerp(item1, item2, new_lerp_val);

                    delay_buf[pos0] = (item0 + item1) * 0.5f;
                    delay_buf[pos1] = (item1 + item2) * 0.5f;
                }
            }
            else
            {
                // Period length is an integer
                const int32_t pos1 = (int32_t)pos;
                const int32_t pos2 = 0;

                const float item1 = delay_buf[pos1];
                const float item2 = delay_buf[pos2];
                value = (float)lerp(item1, item2, lerp_val);

                delay_buf[pos1] = (item1 + item2) * 0.5f;
            }

            out_buf[i] = value * scale;

            pos = pos + 1 - used_buf_length;
            rassert(pos >= 0);
            rassert(pos < used_buf_length);

            ++i;
        }
    }

    ks_vstate->buf_pos = pos;

    return buf_stop;
}


void Ks_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Ks_vstate_render_voice;

    Ks_vstate* ks_vstate = (Ks_vstate*)vstate;

    ks_vstate->init_pitch = NAN;
    ks_vstate->buf_pos = 0;

    for (int i = 0; i < DELAY_BUF_LEN_MAX; ++i)
        ks_vstate->delay_buf[i] = 0;

    return;
}


