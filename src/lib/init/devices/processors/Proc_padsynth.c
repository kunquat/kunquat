

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


#include <init/devices/processors/Proc_padsynth.h>

#include <containers/Vector.h>
#include <debug/assert.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <mathnum/irfft.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <player/devices/processors/Padsynth_state.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static Set_padsynth_params_func Proc_padsynth_set_params;
static Set_bool_func            Proc_padsynth_set_ramp_attack;
static Set_bool_func            Proc_padsynth_set_stereo;

static bool apply_padsynth(Proc_padsynth* padsynth, const Padsynth_params* params);

static void del_Proc_padsynth(Device_impl* dimpl);


Device_impl* new_Proc_padsynth(void)
{
    Proc_padsynth* padsynth = memory_alloc_item(Proc_padsynth);
    if (padsynth == NULL)
        return NULL;

    padsynth->random = NULL;
    padsynth->sample = NULL;
    padsynth->is_ramp_attack_enabled = true;
    padsynth->is_stereo_enabled = false;

    if (!Device_impl_init(&padsynth->parent, del_Proc_padsynth))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    padsynth->parent.get_vstate_size = Padsynth_vstate_get_size;
    padsynth->parent.init_vstate = Padsynth_vstate_init;

    if (!(REGISTER_SET_FIXED_STATE(
                padsynth, padsynth_params, params, "p_ps_params.json", NULL) &&
            REGISTER_SET_FIXED_STATE(
                padsynth, bool, ramp_attack, "p_b_ramp_attack.json", true) &&
            REGISTER_SET_FIXED_STATE(
                padsynth, bool, stereo, "p_b_stereo.json", false)))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    padsynth->random = new_Random();
    if (padsynth->random == NULL)
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    Random_set_context(padsynth->random, "PADsynth");

    // Create a single sample as a temporary solution for now
    float* buf = memory_alloc_items(float, 262144 + 1);
    if (buf == NULL)
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    padsynth->sample = new_Sample_from_buffers(&buf, 1, 262144 + 1);
    if (padsynth->sample == NULL)
    {
        memory_free(buf);
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    if (!apply_padsynth(padsynth, NULL))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    return &padsynth->parent;
}


static bool Proc_padsynth_set_params(
        Device_impl* dimpl, const Key_indices indices, const Padsynth_params* params)
{
    assert(dimpl != NULL);
    ignore(indices);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;

    return apply_padsynth(padsynth, params);
}


static bool Proc_padsynth_set_ramp_attack(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->is_ramp_attack_enabled = enabled;

    return true;
}


static bool Proc_padsynth_set_stereo(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->is_stereo_enabled = enabled;

    return true;
}


static double profile(double freq_i, double bandwidth_i)
{
    const double x = freq_i / bandwidth_i;
    return exp(-x * x) / bandwidth_i;
}


static bool apply_padsynth(Proc_padsynth* padsynth, const Padsynth_params* params)
{
    assert(padsynth != NULL);

    int32_t sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    int32_t audio_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    double bandwidth_base = PADSYNTH_DEFAULT_BANDWIDTH_BASE;
    double bandwidth_scale = PADSYNTH_DEFAULT_BANDWIDTH_SCALE;
    if (params != NULL)
    {
        sample_length = params->sample_length;
        audio_rate = params->audio_rate;
        bandwidth_base = params->bandwidth_base;
        bandwidth_scale = params->bandwidth_scale;
    }

    const int32_t buf_length = sample_length / 2;

    double* freq_amp = memory_alloc_items(double, buf_length);
    double* freq_phase = memory_alloc_items(double, buf_length);
    if (freq_amp == NULL || freq_phase == NULL)
    {
        memory_free(freq_amp);
        memory_free(freq_phase);
        return false;
    }

    for (int32_t i = 0; i < buf_length; ++i)
    {
        freq_amp[i] = 0;
        freq_phase[i] = 0;
    }

    // Apply harmonics
    if (params != NULL)
    {
        static const double freq = 440; // TODO: add multi-sample support

        const int32_t nyquist = audio_rate / 2;

        for (size_t h = 0; h < Vector_size(params->harmonics); ++h)
        {
            const Padsynth_harmonic* harmonic = Vector_get_ref(params->harmonics, h);

            // Skip harmonics that are not representable
            // NOTE: this only considers the center frequency
            if (freq * harmonic->freq_mul >= nyquist)
                continue;

            const double bandwidth_Hz =
                (exp2(bandwidth_base / 1200.0) - 1.0) *
                freq *
                pow(harmonic->freq_mul, bandwidth_scale);
            const double bandwidth_i = bandwidth_Hz / (2.0 * audio_rate);
            const double freq_i = freq * harmonic->freq_mul / audio_rate;

            for (int32_t i = 0; i < buf_length; ++i)
            {
                const double harmonic_profile =
                    profile((i / (double)sample_length) - freq_i, bandwidth_i);
                freq_amp[i] += harmonic_profile * harmonic->amplitude;
            }
        }
    }
    else
    {
        static const double freq = 440;
        const double bandwidth_Hz = (exp2(bandwidth_base / 1200.0) - 1.0) * freq;
        const double bandwidth_i = bandwidth_Hz / (2.0 * audio_rate);
        const double freq_i = freq / audio_rate;

        for (int32_t i = 0; i < buf_length; ++i)
        {
            const double harmonic_profile =
                profile((i / (double)sample_length) - freq_i, bandwidth_i);
            freq_amp[i] += harmonic_profile;
        }
    }

    // Add randomised phases
    {
        Random_reset(padsynth->random);
        for (int32_t i = 0; i < buf_length; ++i)
            freq_phase[i] = Random_get_float_lb(padsynth->random) * 2 * PI;
    }

    // Set up frequencies in half-complex representation
    float* buf = Sample_get_buffer(padsynth->sample, 0);

    buf[0] = 0;
    buf[buf_length] = 0;
    for (int32_t i = 1; i < buf_length; ++i)
    {
        buf[i] = freq_amp[i] * cos(freq_phase[i]);
        buf[sample_length - i] = freq_amp[i] * sin(freq_phase[i]);
    }

    // Apply IFFT
    irfft(buf, sample_length);

    // Normalise
    {
        float max_abs = 0.0f;
        for (int32_t i = 0; i < sample_length; ++i)
            max_abs = max(max_abs, fabs(buf[i]));

        const float scale = 1.0f / max_abs;
        for (int32_t i = 0; i < sample_length; ++i)
            buf[i] *= scale;
    }

    // Duplicate first frame (for interpolation code)
    buf[sample_length] = buf[0];

    memory_free(freq_amp);
    memory_free(freq_phase);

    return true;
}


static void del_Proc_padsynth(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    del_Sample(padsynth->sample);
    del_Random(padsynth->random);
    memory_free(padsynth);

    return;
}


