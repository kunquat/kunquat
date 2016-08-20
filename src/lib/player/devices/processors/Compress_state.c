

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


#include <player/devices/processors/Compress_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_compress.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MIN_LEVEL 0.0009765625 // -60 dB


typedef struct Compress_state
{
    float level;
} Compress_state;


static void Compress_state_init(Compress_state* cstate)
{
    rassert(cstate != NULL);
    cstate->level = MIN_LEVEL;
    return;
}


static void Compress_states_update(
        Compress_state cstates[2],
        const Proc_compress* compress,
        Work_buffer* gain_wb,
        Work_buffer* level_wbs[2],
        const Work_buffer* in_wbs[2],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    rassert(cstates != NULL);
    rassert(compress != NULL);
    rassert(gain_wb != NULL);
    rassert(level_wbs != NULL);
    rassert(level_wbs[0] != NULL);
    rassert(level_wbs[1] != NULL);
    rassert(in_wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    // Get levels
    const float attack_mul =
        (float)dB_to_scale(6 / (compress->attack * 0.001 * audio_rate));
    const float release_mul =
        (float)dB_to_scale(-6 / (compress->release * 0.001 * audio_rate));

    for (int ch = 0; ch < 2; ++ch)
    {
        Compress_state* cstate = &cstates[ch];
        rassert(cstate != NULL);

        float level = cstate->level;

        float* levels = Work_buffer_get_contents_mut(level_wbs[ch]);
        const float* in = Work_buffer_get_contents(in_wbs[ch]);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float in_abs = fabsf(in[i]);
            if (in_abs > level)
            {
                level *= attack_mul;
                level = min(level, in_abs);
            }
            else
            {
                level *= release_mul;
                level = max(level, max((float)MIN_LEVEL, in_abs));
            }

            levels[i] = level;
        }

        if (ch == 0)
        {
            //fprintf(stdout, "%.7f\n", level);
        }

        cstate->level = level;
    }

    if ((in_wbs[0] != NULL) && (in_wbs[1] != NULL))
    {
        // Get maximum levels
        for (int32_t i = buf_start; i < buf_stop; ++i)
            level_wbs[0] = max(level_wbs[0], level_wbs[1]);
    }

    const Work_buffer* applied_levels_wb =
        (in_wbs[0] == NULL) ? level_wbs[1] : level_wbs[0];
    const float* applied_levels = Work_buffer_get_contents(applied_levels_wb);

    Work_buffer_clear(gain_wb, buf_start, buf_stop);
    float* gains = Work_buffer_get_contents_mut(gain_wb);

    for (int32_t i = buf_start; i < buf_stop; ++i)
        gains[i] = 1.0f;

    if (compress->upward_enabled)
    {
        // Apply upward compression
        const double upward_threshold_dB =
            compress->downward_enabled
            ? min(compress->upward_threshold, compress->downward_threshold)
            : compress->upward_threshold;
        const float threshold = (float)dB_to_scale(upward_threshold_dB);
        const float inv_ratio = (float)(1.0 / compress->upward_ratio);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float level = applied_levels[i];
            if (level < threshold)
            {
                const float diff = threshold / level;
                gains[i] = threshold / (powf(diff, inv_ratio) * level);
            }
        }
    }

    if (compress->downward_enabled)
    {
        // Apply downward compression
        const float threshold = (float)dB_to_scale(compress->downward_threshold);
        const float inv_ratio = (float)(1.0 / compress->downward_ratio);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float level = applied_levels[i];
            if (level > threshold)
            {
                const float diff = level / threshold;
                gains[i] = (threshold * powf(diff, inv_ratio)) / level;
            }
        }
    }

    return;
}


static void write_audio(
        Work_buffer* out_wbs[2],
        const Work_buffer* gain_wb,
        const Work_buffer* in_wbs[2],
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(out_wbs != NULL);
    rassert(gain_wb != NULL);
    rassert(in_wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    const float* gains = Work_buffer_get_contents(gain_wb);

    for (int ch = 0; ch < 2; ++ch)
    {
        if (out_wbs[ch] == NULL || in_wbs[ch] == NULL)
            continue;

        float* out = Work_buffer_get_contents_mut(out_wbs[ch]);
        const float* in = Work_buffer_get_contents(in_wbs[ch]);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            out[i] = in[i] * gains[i];
    }

    return;
}


typedef struct Compress_pstate
{
    Proc_state parent;

    Compress_state cstates[2];
} Compress_pstate;


static void del_Compress_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Compress_pstate* cpstate = (Compress_pstate*)dstate;
    memory_free(cpstate);

    return;
}


static void Compress_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Compress_pstate* cpstate = (Compress_pstate*)dstate;

    for (int ch = 0; ch < 2; ++ch)
        Compress_state_init(&cpstate->cstates[ch]);

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_GAIN,
    PORT_OUT_COUNT
};


static const int COMPRESS_WB_GAIN = WORK_BUFFER_IMPL_1;
static const int COMPRESS_WB_LEVEL_L = WORK_BUFFER_IMPL_2;
static const int COMPRESS_WB_LEVEL_R = WORK_BUFFER_IMPL_3;


static void Compress_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_compress* compress = (const Proc_compress*)dstate->device->dimpl;

    Compress_pstate* cpstate = (Compress_pstate*)dstate;

    // Get audio input buffers
    const Work_buffer* in_wbs[2] =
    {
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_AUDIO_L),
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_AUDIO_R),
    };

    // Get audio output buffers
    Work_buffer* out_wbs[2] =
    {
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    // Get level buffers
    Work_buffer* level_wbs[2] =
    {
        Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_LEVEL_L),
        Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_LEVEL_R),
    };

    // Get gain buffer
    Work_buffer* gain_wb =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, PORT_OUT_GAIN);
    if (gain_wb == NULL)
        gain_wb = Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_GAIN);

    Compress_states_update(
            cpstate->cstates,
            compress,
            gain_wb,
            level_wbs,
            in_wbs,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    write_audio(out_wbs, gain_wb, in_wbs, buf_start, buf_stop);

    return;
}


Device_state* new_Compress_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Compress_pstate* cpstate = memory_alloc_item(Compress_pstate);
    if (cpstate == NULL)
        return NULL;

    if (!Proc_state_init(&cpstate->parent, device, audio_rate, audio_buffer_size))
    {
        del_Device_state((Device_state*)cpstate);
        return NULL;
    }

    cpstate->parent.destroy = del_Compress_pstate;
    cpstate->parent.reset = Compress_pstate_reset;
    cpstate->parent.render_mixed = Compress_pstate_render_mixed;

    for (int ch = 0; ch < 2; ++ch)
        Compress_state_init(&cpstate->cstates[ch]);

    return (Device_state*)cpstate;
}


typedef struct Compress_vstate
{
    Voice_state parent;

    Compress_state cstates[2];
} Compress_vstate;


int32_t Compress_vstate_get_size(void)
{
    return sizeof(Compress_vstate);
}


static int32_t Compress_vstate_render_voice(
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
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Device_state* dstate = &proc_state->parent;
    const Proc_compress* compress =
        (const Proc_compress*)proc_state->parent.device->dimpl;

    Compress_vstate* cvstate = (Compress_vstate*)vstate;

    // Get audio input buffers
    const Work_buffer* in_wbs[2] =
    {
        Proc_state_get_voice_buffer(
                proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_AUDIO_L),
        Proc_state_get_voice_buffer(
                proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_AUDIO_R),
    };

    // Get audio output buffers
    Work_buffer* out_wbs[2] =
    {
        Proc_state_get_voice_buffer_mut(
                proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Proc_state_get_voice_buffer_mut(
                proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    // Get level buffers
    Work_buffer* level_wbs[2] =
    {
        Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_LEVEL_L),
        Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_LEVEL_R),
    };

    // Get gain buffer
    Work_buffer* gain_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_GAIN);
    if (gain_wb == NULL)
        gain_wb = Work_buffers_get_buffer_mut(wbs, COMPRESS_WB_GAIN);

    Compress_states_update(
            cvstate->cstates,
            compress,
            gain_wb,
            level_wbs,
            in_wbs,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    write_audio(out_wbs, gain_wb, in_wbs, buf_start, buf_stop);

    return buf_stop;
}


void Compress_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Compress_vstate_render_voice;

    Compress_vstate* cvstate = (Compress_vstate*)vstate;

    for (int ch = 0; ch < 2; ++ch)
        Compress_state_init(&cvstate->cstates[ch]);

    return;
}


