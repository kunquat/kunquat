

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Add_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_add.h>
#include <mathnum/common.h>
#include <player/devices/processors/Proc_utils.h>


#define ADD_BASE_FUNC_SIZE_MASK (ADD_BASE_FUNC_SIZE - 1)


typedef struct Add_tone_state
{
    double phase[2];
} Add_tone_state;


typedef struct Add_vstate
{
    Voice_state parent;
    int tone_limit;
    Add_tone_state tones[ADD_TONES_MAX];
} Add_vstate;


size_t Add_vstate_get_size(void)
{
    return sizeof(Add_vstate);
}


static int32_t Add_vstate_render_voice(
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
    const Processor* proc = (const Processor*)proc_state->parent.device;
    const Proc_add* add = (Proc_add*)proc_state->parent.device->dimpl;
    Add_vstate* add_state = (Add_vstate*)vstate;
    assert(is_p2(ADD_BASE_FUNC_SIZE));

    // Get actual pitches
    const Cond_work_buffer* actual_pitches = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_PITCHES),
            440,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH));

    // Get actual forces
    const Cond_work_buffer* actual_forces = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_FORCES),
            1,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE));

    // Get output buffer for writing
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);
    Audio_buffer_clear(out_buffer, buf_start, buf_stop);

    float* out_values[] =
    {
        Audio_buffer_get_buffer(out_buffer, 0),
        Audio_buffer_get_buffer(out_buffer, 1),
    };

    // Get phase modulation signal
    static const int ADD_WORK_BUFFER_MOD_L = WORK_BUFFER_IMPL_1;
    static const int ADD_WORK_BUFFER_MOD_R = WORK_BUFFER_IMPL_2;

    float* mod_values[] =
    {
        Work_buffers_get_buffer_contents_mut(wbs, ADD_WORK_BUFFER_MOD_L),
        Work_buffers_get_buffer_contents_mut(wbs, ADD_WORK_BUFFER_MOD_R),
    };

    Audio_buffer* mod_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);

    if (mod_buffer != NULL)
    {
        // Copy from the input voice buffer
        // XXX: not sure if the best way to handle this...
        const float* mod_in_values[] =
        {
            Audio_buffer_get_buffer(mod_buffer, 0),
            Audio_buffer_get_buffer(mod_buffer, 1),
        };

        for (int ch = 0; ch < 2; ++ch)
        {
            const float* mod_in_values_ch = mod_in_values[ch];
            float* mod_values_ch = mod_values[ch];

            for (int32_t i = buf_start; i < buf_stop; ++i)
                mod_values_ch[i] = mod_in_values_ch[i];
        }
    }
    else
    {
        // Fill with zeroes if no modulation signal
        for (int ch = 0; ch < 2; ++ch)
        {
            float* mod_values_ch = mod_values[ch];

            for (int32_t i = buf_start; i < buf_stop; ++i)
                mod_values_ch[i] = 0;
        }
    }

    // Add base waveform tones
    const double inv_audio_rate = 1.0 / dstate->audio_rate;

    const float* base = Sample_get_buffer(add->base, 0);

    for (int h = 0; h < add_state->tone_limit; ++h)
    {
        const Add_tone* tone = &add->tones[h];
        const double pitch_factor = tone->pitch_factor;
        const double volume_factor = tone->volume_factor;

        if ((pitch_factor <= 0) || (volume_factor <= 0))
            continue;

        const double pannings[] =
        {
            -tone->panning,
            tone->panning,
        };

        const double pitch_factor_inv_audio_rate = pitch_factor * inv_audio_rate;

        Add_tone_state* tone_state = &add_state->tones[h];

        for (int32_t ch = 0; ch < 2; ++ch)
        {
            const double panning_factor = 1 + pannings[ch];
            const float* mod_values_ch = mod_values[ch];

            double phase = tone_state->phase[ch];
            float* out_values_ch = out_values[ch];

            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const float actual_pitch = Cond_work_buffer_get_value(
                        actual_pitches, i);
                const float mod_val = mod_values_ch[i];

                // Note: + mod_val is specific to phase modulation
                const double actual_phase = phase + mod_val;
                const double pos = actual_phase * ADD_BASE_FUNC_SIZE;

                // Note: direct cast of negative doubles to uint32_t is undefined
                const uint32_t pos1 = (uint32_t)(int32_t)pos & ADD_BASE_FUNC_SIZE_MASK;
                const uint32_t pos2 = pos1 + 1;

                const float item1 = base[pos1];
                const float item_diff = base[pos2] - item1;
                const double lerp_val = pos - floor(pos);
                const double value =
                    (item1 + (lerp_val * item_diff)) * volume_factor * panning_factor;

                out_values_ch[i] += value;

                phase += actual_pitch * pitch_factor_inv_audio_rate;

                // Normalise to range [0, 1)
                // phase is usually < 2, so this is faster than subtracting floor(phase)
                while (phase >= 1)
                    phase -= 1;
            }

            tone_state->phase[ch] = phase;
        }
    }

    // Apply actual force
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_force = Cond_work_buffer_get_value(
                actual_forces, i);
        out_values[0][i] *= actual_force;
        out_values[1][i] *= actual_force;
    }

    if (add->is_ramp_attack_enabled)
        Proc_ramp_attack(
                proc, vstate, out_buffer, 2, dstate->audio_rate, buf_start, buf_stop);

    vstate->pos = 1; // XXX: hackish

    return buf_stop;
}


void Add_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Add_vstate_render_voice;

    Proc_add* add = (Proc_add*)proc_state->parent.device->dimpl;
    Add_vstate* add_state = (Add_vstate*)vstate;

    add_state->tone_limit = 0;

    for (int h = 0; h < ADD_TONES_MAX; ++h)
    {
        if (add->tones[h].pitch_factor <= 0 ||
                add->tones[h].volume_factor <= 0)
            continue;

        add_state->tone_limit = h + 1;
        for (int ch = 0; ch < 2; ++ch)
            add_state->tones[h].phase[ch] = 0;
    }

    return;
}

