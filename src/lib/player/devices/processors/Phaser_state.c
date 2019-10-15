

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Phaser_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_phaser.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fast_exp2.h>
#include <mathnum/fast_tan.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Filter_utils.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define PHASER_STAGES_MAX 32


typedef struct Phaser_allpass_state
{
    float s1;
    float s2;
} Phaser_allpass_state;


typedef struct Phaser_ch_state
{
    Phaser_allpass_state states[PHASER_STAGES_MAX];
} Phaser_ch_state;


typedef struct Phaser_impl
{
    Phaser_ch_state ch_states[2];
} Phaser_impl;


typedef struct Phaser_pstate
{
    Proc_state parent;

    Phaser_impl impl;
} Phaser_pstate;


static void Phaser_impl_init(Phaser_impl* phaser)
{
    rassert(phaser != NULL);

    for (int ch = 0; ch < 2; ++ch)
    {
        Phaser_ch_state* ch_state = &phaser->ch_states[ch];

        for (int i = 0; i < PHASER_STAGES_MAX; ++i)
        {
            ch_state->states[i].s1 = 0;
            ch_state->states[i].s2 = 0;
        }
    }

    return;
}


static const int PHASER_WB_CUTOFF       = WORK_BUFFER_IMPL_1;
static const int PHASER_WB_BANDWIDTH    = WORK_BUFFER_IMPL_2;
static const int PHASER_WB_DRY_WET      = WORK_BUFFER_IMPL_3;


static void Phaser_impl_update(
        Phaser_impl* phaser_impl,
        const Proc_phaser* phaser,
        const Work_buffers* wbs,
        const Work_buffer* in_wbs[2],
        Work_buffer* out_wbs[2],
        const Work_buffer* cutoff_wb,
        const Work_buffer* bandwidth_wb,
        const Work_buffer* dry_wet_wb,
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(phaser_impl != NULL);
    rassert(phaser != NULL);
    rassert(wbs != NULL);
    rassert(in_wbs != NULL);
    rassert(out_wbs != NULL);
    rassert(frame_count > 0);

    const float* in_data[2] =
    {
        in_wbs[0] ? Work_buffer_get_contents(in_wbs[0]) : NULL,
        in_wbs[1] ? Work_buffer_get_contents(in_wbs[1]) : NULL,
    };

    // Get transformed cutoff values
    Work_buffer* dest_cutoff_wb = Work_buffers_get_buffer_mut(wbs, PHASER_WB_CUTOFF);
    transform_cutoff(dest_cutoff_wb, cutoff_wb, phaser->cutoff, frame_count, audio_rate);

    const float* cutoffs = Work_buffer_get_contents(dest_cutoff_wb);

    const int stage_count = phaser->stage_count;

    // Get bandwidth input
    // R = sinh(bandwidth * 2 * ln 2 / 2) / cot(PI / (2 * stage_count))

    const float ln2 = logf(2.0f);
    const float bw_cot_mult =
        1.0f / (float)(-tan((PI / (2 * max(stage_count, 2))) + PI / 2.0));

    Work_buffer* dest_bandwidth_wb =
        Work_buffers_get_buffer_mut(wbs, PHASER_WB_BANDWIDTH);
    {
        int32_t const_start = 0;
        float fixed_bw = (float)phaser->bandwidth;

        float* bws = Work_buffer_get_contents_mut(dest_bandwidth_wb);

        if (Work_buffer_is_valid(bandwidth_wb))
        {
            const_start = Work_buffer_get_const_start(bandwidth_wb);
            const_start = min(const_start, frame_count);

            const float* src_bws = Work_buffer_get_contents(bandwidth_wb);
            for (int32_t i = 0; i < const_start; ++i)
            {
                float bw = src_bws[i];
                bw = clamp(bw, (float)PHASER_BANDWIDTH_MIN, (float)PHASER_BANDWIDTH_MAX);

                // e^x = 2^(x / ln 2), so we can eliminate the ln 2 factor here
                const float scaled_bw = bw;
                float sinh_bw =
                    (float)(fast_exp2(scaled_bw) - fast_exp2(-scaled_bw)) * 0.5f;
                bws[i] = sinh_bw * bw_cot_mult;
            }

            if (const_start < frame_count)
                fixed_bw = clamp(
                        src_bws[const_start],
                        (float)PHASER_BANDWIDTH_MIN,
                        (float)PHASER_BANDWIDTH_MAX);
        }

        if (const_start < frame_count)
        {
            const float scaled_bw = fixed_bw * ln2;
            float sinh_bw = sinhf(scaled_bw);
            float fixed_res = sinh_bw * bw_cot_mult;

            for (int32_t i = const_start; i < frame_count; ++i)
                bws[i] = fixed_res;
        }
    }

    const float* resonances = Work_buffer_get_contents(dest_bandwidth_wb);

    // Get dry/wet ratio input
    Work_buffer* clamped_dry_wet_wb =
        Work_buffers_get_buffer_mut(wbs, PHASER_WB_DRY_WET);
    if (Work_buffer_is_valid(dry_wet_wb))
    {
        const float* unclamped_dry_wets = Work_buffer_get_contents(dry_wet_wb);
        float* dry_wets = Work_buffer_get_contents_mut(clamped_dry_wet_wb);
        for (int32_t i = 0; i < frame_count; ++i)
            dry_wets[i] = clamp(unclamped_dry_wets[i], 0.0f, 1.0f);
    }
    else
    {
        float fixed_dry_wet = (float)phaser->dry_wet_ratio;

        float* dry_wets = Work_buffer_get_contents_mut(clamped_dry_wet_wb);
        for (int32_t i = 0; i < frame_count; ++i)
            dry_wets[i] = fixed_dry_wet;
    }

    const float* dry_wets = Work_buffer_get_contents(clamped_dry_wet_wb);

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* out_wb = out_wbs[ch];
        if (out_wb == NULL)
            continue;

        const float* in = in_data[ch];
        if (in == NULL)
            continue;

        Phaser_ch_state* ch_state = &phaser_impl->ch_states[ch];

        // Clear unused allpass states in case they contain obsolete values
        for (int si = stage_count; si < PHASER_STAGES_MAX; ++si)
        {
            ch_state->states[si].s1 = 0;
            ch_state->states[si].s2 = 0;
        }

        float* out = Work_buffer_get_contents_mut(out_wb);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float input = *in++;
            const float cutoff = cutoffs[i];
            const float res = resonances[i];
            const float dry_wet = dry_wets[i] * 0.5f;

            const float hp_mult = 1.0f / (1.0f + (res * cutoff) + (cutoff * cutoff));
            const float rpc = res + cutoff;

            float stage_value = input;

            for (int si = 0; si < stage_count; ++si)
            {
                Phaser_allpass_state* restrict ap = &ch_state->states[si];

                const float hp_sample =
                    (stage_value - (ap->s1 * rpc) - ap->s2) * hp_mult;

                const float input_1 = cutoff * hp_sample;
                const float bp_sample = input_1 + ap->s1;
                ap->s1 = input_1 + bp_sample;

                const float input_2 = cutoff * bp_sample;
                const float lp_sample = input_2 + ap->s2;
                ap->s2 = input_2 + lp_sample;

                stage_value = stage_value - (bp_sample * 2.0f * res);
            }

            *out++ = (input * dry_wet) + (stage_value * (1 - dry_wet));
        }
    }

    return;
}


static void del_Phaser_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Phaser_pstate* ppstate = (Phaser_pstate*)dstate;
    memory_free(ppstate);

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_CUTOFF,
    PORT_IN_NOTCH_SEP,
    PORT_IN_DRY_WET,
    PORT_IN_COUNT,
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static void Phaser_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(wbs != NULL);
    rassert(tempo > 0);

    // Get audio buffers
    const Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        in_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);
        if (!Work_buffer_is_valid(in_wbs[ch]))
            in_wbs[ch] = NULL;
    };

    if ((in_wbs[0] == NULL) && (in_wbs[1] == NULL))
        return;

    // Get output
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);
    if ((out_wbs[0] == NULL) && (out_wbs[1] == NULL))
        return;

    Phaser_pstate* ppstate = (Phaser_pstate*)dstate;
    const Proc_phaser* phaser = (const Proc_phaser*)dstate->device->dimpl;

    Phaser_impl_update(
            &ppstate->impl,
            phaser,
            wbs,
            in_wbs,
            out_wbs,
            Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF),
            Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_NOTCH_SEP),
            Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_DRY_WET),
            frame_count,
            dstate->audio_rate);

    return;
}


Device_state* new_Phaser_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size > 0);

    Phaser_pstate* ppstate = memory_alloc_item(Phaser_pstate);
    if (ppstate == NULL)
        return NULL;

    if (!Proc_state_init(&ppstate->parent, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(&ppstate->parent.parent);
        return NULL;
    }

    Phaser_impl_init(&ppstate->impl);

    ppstate->parent.destroy = del_Phaser_pstate;
    ppstate->parent.render_mixed = Phaser_pstate_render_mixed;

    return &ppstate->parent.parent;
}


