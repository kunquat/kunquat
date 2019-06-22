

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Bitcrusher_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_bitcrusher.h>
#include <mathnum/common.h>
#include <mathnum/fast_exp2.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Bitcrusher_state_impl
{
    double hold_timer[2];
    float hold_value[2];
} Bitcrusher_state_impl;


static double get_hold(double cutoff, int32_t audio_rate)
{
    rassert(isfinite(cutoff));
    rassert(audio_rate > 0);

    return audio_rate / (exp2(cutoff / 12.0) * 220);
}


static double get_hold_fast(double cutoff, int32_t audio_rate)
{
    dassert(isfinite(cutoff));
    dassert(audio_rate > 0);

    return audio_rate / (fast_exp2(cutoff / 12.0) * 220);
}


static void Bitcrusher_state_impl_init(Bitcrusher_state_impl* state)
{
    rassert(state != NULL);

    for (int ch = 0; ch < 2; ++ch)
    {
        state->hold_timer[ch] = INFINITY;
        state->hold_value[ch] = 0;
    }

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_CUTOFF,
    PORT_IN_RESOLUTION,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int BC_WB_HOLDS = WORK_BUFFER_IMPL_1;
static const int BC_WB_MULTS = WORK_BUFFER_IMPL_2;


static void Bitcrusher_state_impl_render(
        Bitcrusher_state_impl* state,
        const Proc_bitcrusher* bc,
        const Work_buffers* wbs,
        Work_buffer* cutoff_wb,
        Work_buffer* resolution_wb,
        Work_buffer* in_buffers[2],
        Work_buffer* out_buffers[2],
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(state != NULL);
    rassert(bc != NULL);
    rassert(wbs != NULL);
    rassert(in_buffers != NULL);
    rassert(out_buffers != NULL);
    rassert(frame_count > 0);
    rassert(audio_rate > 0);

    // Get hold durations
    Work_buffer* holds_wb = Work_buffers_get_buffer_mut(wbs, BC_WB_HOLDS, 1);
    if ((cutoff_wb == NULL) || !Work_buffer_is_valid(cutoff_wb, 0))
    {
        const float hold = (float)get_hold(bc->cutoff, audio_rate);
        if (hold <= 1.0f)
        {
            holds_wb = NULL;
        }
        else
        {
            float* holds = Work_buffer_get_contents_mut(holds_wb, 0);
            for (int32_t i = 0; i < frame_count; ++i)
                holds[i] = hold;
            Work_buffer_set_const_start(holds_wb, 0, 0);
        }
    }
    else
    {
        const int32_t const_start = Work_buffer_get_const_start(cutoff_wb, 0);

        const float* cutoffs = Work_buffer_get_contents(cutoff_wb, 0);
        float* holds = Work_buffer_get_contents_mut(holds_wb, 0);

        for (int32_t i = 0; i < const_start; ++i)
            holds[i] = (float)get_hold_fast(cutoffs[i], audio_rate);

        if (const_start < frame_count)
        {
            const float hold = (float)get_hold(cutoffs[const_start], audio_rate);
            for (int32_t i = const_start; i < frame_count; ++i)
                holds[i] = hold;
        }

        Work_buffer_set_const_start(holds_wb, 0, const_start);
    }

    // Perform audio rate reduction
    if (holds_wb != NULL)
    {
        const float* holds = Work_buffer_get_contents(holds_wb, 0);

        for (int ch = 0; ch < 2; ++ch)
        {
            const Work_buffer* in_wb = in_buffers[ch];
            Work_buffer* out_wb = out_buffers[ch];
            if ((in_wb == NULL) || (out_wb == NULL))
                continue;

            const float* in = Work_buffer_get_contents(in_wb, 0);
            float* out = Work_buffer_get_contents_mut(out_wb, 0);

            double hold_timer = state->hold_timer[ch];
            float hold_value = state->hold_value[ch];

            for (int32_t i = 0; i < frame_count; ++i)
            {
                hold_timer += 1.0;

                const float hold = holds[i];
                if (hold_timer >= hold)
                {
                    double excess = hold_timer - hold;
                    excess = min(1.0, excess);

                    const float value = lerp(hold_value, in[i], (float)excess);
                    out[i] = value;

                    hold_value = in[i];
                    hold_timer = excess;

                    continue;
                }

                out[i] = hold_value;
            }

            state->hold_timer[ch] = hold_timer;
            state->hold_value[ch] = hold_value;
        }
    }

    // Get resolution reduction multipliers
    Work_buffer* mults_wb = Work_buffers_get_buffer_mut(wbs, BC_WB_MULTS, 1);
    if ((resolution_wb == NULL) || !Work_buffer_is_valid(resolution_wb, 0))
    {
        if (bc->resolution >= bc->res_ignore_min)
        {
            mults_wb = NULL;
        }
        else
        {
            const float mult = (float)exp2(bc->resolution);

            float* mults = Work_buffer_get_contents_mut(mults_wb, 0);
            for (int32_t i = 0; i < frame_count; ++i)
                mults[i] = mult;

            Work_buffer_set_const_start(mults_wb, 0, 0);
        }
    }
    else
    {
        const int32_t const_start = Work_buffer_get_const_start(resolution_wb, 0);

        const float* res_buf = Work_buffer_get_contents(resolution_wb, 0);
        float* mults = Work_buffer_get_contents_mut(mults_wb, 0);

        for (int32_t i = 0; i < const_start; ++i)
            mults[i] = (float)fast_exp2(max(1, res_buf[i]));

        if (const_start < frame_count)
        {
            const float mult = (float)exp2(max(1, res_buf[const_start]));
            for (int32_t i = const_start; i < frame_count; ++i)
                mults[i] = mult;
        }

        Work_buffer_set_const_start(mults_wb, 0, const_start);
    }

    // Perform resolution reduction
    if (mults_wb != NULL)
    {
        const float min_ignore_mult = (float)exp2(bc->res_ignore_min);

        const float* mults = Work_buffer_get_contents(mults_wb, 0);

        if (holds_wb == NULL)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                const Work_buffer* in_wb = in_buffers[ch];
                Work_buffer* out_wb = out_buffers[ch];
                if ((in_wb == NULL) || (out_wb == NULL))
                    continue;

                const float* in = Work_buffer_get_contents(in_wb, 0);
                float* out = Work_buffer_get_contents_mut(out_wb, 0);

                for (int32_t i = 0; i < frame_count; ++i)
                {
                    const float mult = mults[i];
                    if (mult < min_ignore_mult)
                    {
                        const float scaled = (((in[i] + 1) * 0.5f) * mult);
                        const float floored = floorf(scaled);
                        out[i] = ((floored / mult) * 2.0f) - 1;
                    }
                    else
                    {
                        out[i] = in[i];
                    }
                }
            }
        }
        else
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                Work_buffer* wb = out_buffers[ch];
                if (wb == NULL)
                    continue;

                float* out = Work_buffer_get_contents_mut(wb, 0);

                for (int32_t i = 0; i < frame_count; ++i)
                {
                    const float mult = mults[i];
                    if (mult < min_ignore_mult)
                    {
                        const float scaled = (((out[i] + 1) * 0.5f) * mult);
                        const float floored = floorf(scaled);
                        out[i] = ((floored / mult) * 2.0f) - 1;
                    }
                }
            }
        }
    }

    if ((holds_wb == NULL) && (mults_wb == NULL))
    {
        // Both operations are disabled, so simply copy
        for (int ch = 0; ch < 2; ++ch)
        {
            const Work_buffer* in_wb = in_buffers[ch];
            Work_buffer* out_wb = out_buffers[ch];
            if ((in_wb == NULL) || (out_wb == NULL))
                continue;

            Work_buffer_copy(out_wb, 0, in_wb, 0, 0, frame_count);
        }
    }

    return;
}


typedef struct Bitcrusher_pstate
{
    Proc_state parent;
    Bitcrusher_state_impl state_impl;
} Bitcrusher_pstate;


static void Bitcrusher_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Bitcrusher_pstate* bcpstate = (Bitcrusher_pstate*)dstate;
    Bitcrusher_state_impl_init(&bcpstate->state_impl);

    return;
}


static void Bitcrusher_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Bitcrusher_pstate* bcpstate = (Bitcrusher_pstate*)dstate;
    const Proc_bitcrusher* bc = (Proc_bitcrusher*)dstate->device->dimpl;

    // Get parameter inputs
    Work_buffer* cutoff_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    Work_buffer* resolution_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESOLUTION);

    // Get audio inputs
    Work_buffer* in_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_R),
    };
    for (int ch = 0; ch < 2; ++ch)
    {
        if ((in_buffers[ch] != NULL) && !Work_buffer_is_valid(in_buffers[ch], 0))
            in_buffers[ch] = NULL;
    }

    // Get audio outputs
    Work_buffer* out_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    Bitcrusher_state_impl_render(
            &bcpstate->state_impl,
            bc,
            wbs,
            cutoff_wb,
            resolution_wb,
            in_buffers,
            out_buffers,
            frame_count,
            dstate->audio_rate);

    return;
}


Device_state* new_Bitcrusher_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Bitcrusher_pstate* bcpstate = memory_alloc_item(Bitcrusher_pstate);
    if ((bcpstate == NULL) ||
            !Proc_state_init(&bcpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(bcpstate);
        return NULL;
    }

    bcpstate->parent.reset = Bitcrusher_pstate_reset;
    bcpstate->parent.render_mixed = Bitcrusher_pstate_render_mixed;

    return (Device_state*)bcpstate;
}


typedef struct Bitcrusher_vstate
{
    Voice_state parent;
    Bitcrusher_state_impl state_impl;
} Bitcrusher_vstate;


int32_t Bitcrusher_vstate_get_size(void)
{
    return sizeof(Bitcrusher_vstate);
}


int32_t Bitcrusher_vstate_render_voice(
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
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Bitcrusher_vstate* bcvstate = (Bitcrusher_vstate*)vstate;

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_bitcrusher* bc = (Proc_bitcrusher*)dstate->device->dimpl;

    // Get parameter inputs
    Work_buffer* cutoff_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    Work_buffer* resolution_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESOLUTION);

    // Get audio inputs
    Work_buffer* in_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_R),
    };
    for (int ch = 0; ch < 2; ++ch)
    {
        if ((in_buffers[ch] != NULL) && !Work_buffer_is_valid(in_buffers[ch], 0))
            in_buffers[ch] = NULL;
    }

    if ((in_buffers[0] == NULL) && (in_buffers[1] == NULL))
    {
        vstate->active = false;
        return 0;
    }

    // Get audio outputs
    Work_buffer* out_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    Bitcrusher_state_impl_render(
            &bcvstate->state_impl,
            bc,
            wbs,
            cutoff_wb,
            resolution_wb,
            in_buffers,
            out_buffers,
            frame_count,
            dstate->audio_rate);

    return frame_count;
}


void Bitcrusher_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Bitcrusher_vstate* bcvstate = (Bitcrusher_vstate*)vstate;
    Bitcrusher_state_impl_init(&bcvstate->state_impl);

    return;
}


