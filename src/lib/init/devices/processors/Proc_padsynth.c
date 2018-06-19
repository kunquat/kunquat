

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


#include <init/devices/processors/Proc_padsynth.h>

#include <containers/AAtree.h>
#include <containers/Vector.h>
#include <debug/assert.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fft.h>
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


struct Padsynth_sample_map
{
    int sample_count;
    int32_t sample_length;
    double min_pitch;
    double max_pitch;
    double centre_pitch;
    AAtree* map;
};


#define PADSYNTH_SAMPLE_ENTRY_KEY(pitch) \
    (&(Padsynth_sample_entry){ .centre_pitch = pitch, .sample = NULL })


static int Padsynth_sample_entry_cmp(
        const Padsynth_sample_entry* entry1, const Padsynth_sample_entry* entry2)
{
    rassert(entry1 != NULL);
    rassert(entry2 != NULL);

    if (entry1->centre_pitch < entry2->centre_pitch)
        return -1;
    else if (entry1->centre_pitch > entry2->centre_pitch)
        return 1;
    return 0;
}


static void del_Padsynth_sample_entry(Padsynth_sample_entry* entry)
{
    if (entry == NULL)
        return;

    del_Sample(entry->sample);
    memory_free(entry);

    return;
}


static void del_Padsynth_sample_map(Padsynth_sample_map* sm);


static Padsynth_sample_map* new_Padsynth_sample_map(
        int sample_count,
        int32_t sample_length,
        double min_pitch,
        double max_pitch,
        double centre_pitch)
{
    rassert(sample_count > 0);
    rassert(sample_count <= 128);
    rassert(sample_length >= PADSYNTH_MIN_SAMPLE_LENGTH);
    rassert(sample_length <= PADSYNTH_MAX_SAMPLE_LENGTH);
    rassert(is_p2(sample_length));
    rassert(isfinite(min_pitch));
    rassert(isfinite(max_pitch));
    rassert(min_pitch <= max_pitch);
    rassert(isfinite(centre_pitch));

    Padsynth_sample_map* sm = memory_alloc_item(Padsynth_sample_map);
    if (sm == NULL)
        return NULL;

    sm->sample_count = sample_count;
    sm->sample_length = sample_length;
    sm->min_pitch = min_pitch;
    sm->max_pitch = max_pitch;
    sm->centre_pitch = centre_pitch;
    sm->map = NULL;

    sm->map = new_AAtree(
            (AAtree_item_cmp*)Padsynth_sample_entry_cmp,
            (AAtree_item_destroy*)del_Padsynth_sample_entry);
    if (sm->map == NULL)
    {
        del_Padsynth_sample_map(sm);
        return NULL;
    }

    for (int i = 0; i < sample_count; ++i)
    {
        float* buf = memory_alloc_items(float, sample_length + 1);
        if (buf == NULL)
        {
            del_Padsynth_sample_map(sm);
            return NULL;
        }

        Sample* sample = new_Sample_from_buffers(&buf, 1, sample_length + 1);
        if (sample == NULL)
        {
            memory_free(buf);
            del_Padsynth_sample_map(sm);
            return NULL;
        }

        Padsynth_sample_entry* entry = memory_alloc_item(Padsynth_sample_entry);
        if (entry == NULL)
        {
            del_Sample(sample);
            del_Padsynth_sample_map(sm);
            return NULL;
        }

        if (sample_count > 1)
            entry->centre_pitch =
                lerp(min_pitch, max_pitch, i / (double)(sample_count - 1));
        else
            entry->centre_pitch = (min_pitch + max_pitch) * 0.5;

        entry->sample = sample;

        if (!AAtree_ins(sm->map, entry))
        {
            del_Padsynth_sample_entry(entry);
            del_Padsynth_sample_map(sm);
            return NULL;
        }
    }

    return sm;
}


static void Padsynth_sample_map_set_centre_pitch(
        Padsynth_sample_map* sm, double centre_pitch)
{
    rassert(sm != NULL);
    rassert(isfinite(centre_pitch));

    sm->centre_pitch = centre_pitch;

    return;
}


static void Padsynth_sample_map_set_pitch_range(
        Padsynth_sample_map* sm, double min_pitch, double max_pitch)
{
    rassert(sm != NULL);
    rassert(isfinite(min_pitch));
    rassert(isfinite(max_pitch));
    rassert(min_pitch <= max_pitch);

    if (sm->min_pitch == min_pitch && sm->max_pitch == max_pitch)
        return;

    sm->min_pitch = min_pitch;
    sm->max_pitch = max_pitch;

    AAiter* iter = AAiter_init(AAITER_AUTO, sm->map);

    const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(-INFINITY);
    Padsynth_sample_entry* entry = AAiter_get_at_least(iter, key);
    if (sm->sample_count == 1)
    {
        rassert(entry != NULL);
        entry->centre_pitch = (min_pitch + max_pitch) * 0.5;

        entry = AAiter_get_next(iter);
    }
    else
    {
        for (int i = 0; i < sm->sample_count; ++i)
        {
            rassert(entry != NULL);
            entry->centre_pitch =
                lerp(min_pitch, max_pitch, i / (double)(sm->sample_count - 1));

            entry = AAiter_get_next(iter);
        }
    }

    rassert(entry == NULL);

    return;
}


const Padsynth_sample_entry* Padsynth_sample_map_get_entry(
        const Padsynth_sample_map* sm, double pitch)
{
    rassert(sm != NULL);
    rassert(isfinite(pitch));

    const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(pitch);
    const Padsynth_sample_entry* prev = AAtree_get_at_most(sm->map, key);
    const Padsynth_sample_entry* next = AAtree_get_at_least(sm->map, key);

    if (prev == NULL)
        return next;
    else if (next == NULL)
        return prev;

    if (fabs(next->centre_pitch - pitch) < fabs(prev->centre_pitch - pitch))
        return next;
    return prev;
}


int32_t Padsynth_sample_map_get_sample_length(const Padsynth_sample_map* sm)
{
    rassert(sm != NULL);
    return sm->sample_length;
}


static void del_Padsynth_sample_map(Padsynth_sample_map* sm)
{
    if (sm == NULL)
        return;

    del_AAtree(sm->map);
    memory_free(sm);

    return;
}


Device_impl* new_Proc_padsynth(void)
{
    Proc_padsynth* padsynth = memory_alloc_item(Proc_padsynth);
    if (padsynth == NULL)
        return NULL;

    padsynth->sample_map = NULL;
    padsynth->is_ramp_attack_enabled = true;
    padsynth->is_stereo_enabled = false;

    if (!Device_impl_init(&padsynth->parent, del_Proc_padsynth))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    padsynth->parent.get_vstate_size = Padsynth_vstate_get_size;
    padsynth->parent.init_vstate = Padsynth_vstate_init;
    padsynth->parent.render_voice = Padsynth_vstate_render_voice;

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

    Random_init(&padsynth->random, "PADsynth");

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
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;

    return apply_padsynth(padsynth, params);
}


static bool Proc_padsynth_set_ramp_attack(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->is_ramp_attack_enabled = enabled;

    return true;
}


static bool Proc_padsynth_set_stereo(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->is_stereo_enabled = enabled;

    return true;
}


static double profile(double freq_i, double bandwidth_i)
{
    double x = freq_i / bandwidth_i;
    x *= x;
    if (x > 27.2972)
        return 0.0;
    return exp(-x) / bandwidth_i;
}


static double get_profile_bound(double bandwidth_i)
{
    rassert(isfinite(bandwidth_i));
    rassert(bandwidth_i > 0);

    return 5.2247 * bandwidth_i; // 5.2247 ~ sqrt(27.2972), see profile() above
}


static void make_padsynth_sample(
        Padsynth_sample_entry* entry,
        Random* random,
        int context_index,
        double* freq_amp,
        double* freq_phase,
        FFT_worker* fw,
        const Padsynth_params* params)
{
    rassert(entry != NULL);
    rassert(random != NULL);
    rassert(freq_amp != NULL);
    rassert(freq_phase != NULL);
    rassert(fw != NULL);

    char context_str[16] = "";
    snprintf(context_str, 16, "PADsynth%hhd", context_index);
    Random_init(random, context_str);

    int32_t sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    if (params != NULL)
        sample_length = params->sample_length;

    const int32_t buf_length = sample_length / 2;

    for (int32_t i = 0; i < buf_length; ++i)
    {
        freq_amp[i] = 0;
        freq_phase[i] = 0;
    }

    if (params != NULL)
    {
        const int32_t audio_rate = params->audio_rate;
        const double bandwidth_base = params->bandwidth_base;
        const double bandwidth_scale = params->bandwidth_scale;

        const double freq = cents_to_Hz(entry->centre_pitch);

        const int32_t nyquist = params->audio_rate / 2;

        for (int64_t h = 0; h < Vector_size(params->harmonics); ++h)
        {
            const Padsynth_harmonic* harmonic = Vector_get_ref(params->harmonics, h);

            // Skip harmonics that are not representable
            // NOTE: this only checks the centre frequency
            if (freq * harmonic->freq_mul >= nyquist)
                continue;

            const double bandwidth_Hz =
                (exp2(bandwidth_base / 1200.0) - 1.0) *
                freq *
                pow(harmonic->freq_mul, bandwidth_scale);
            const double bandwidth_i = bandwidth_Hz / (2.0 * audio_rate);
            const double freq_i = freq * harmonic->freq_mul / audio_rate;

            // Get index range with non-zero data
            const double profile_bound = get_profile_bound(bandwidth_i);
            int32_t buf_start = (int32_t)ceil(sample_length * (freq_i - profile_bound));
            int32_t buf_stop = (int32_t)ceil(sample_length * (freq_i + profile_bound));

            if (buf_start >= buf_length || buf_stop <= 0)
                continue;

            //rassert(profile((buf_start - 1) / (double)sample_length - freq_i, bandwidth_i) == 0.0);
            //rassert(profile((buf_stop + 1) / (double)sample_length - freq_i, bandwidth_i) == 0.0);

            buf_start = max(0, buf_start);
            buf_stop = min(buf_stop, buf_length);
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const double harmonic_profile =
                    profile((i / (double)sample_length) - freq_i, bandwidth_i);
                freq_amp[i] += harmonic_profile * harmonic->amplitude;
            }
        }

        if (params->is_res_env_enabled && (params->res_env != NULL))
        {
            // Apply resonance envelope
            for (int i = 0; i < buf_length; ++i)
            {
                const double env_x = i * 24000.0 / (buf_length - 1);
                const double mult = Envelope_get_value(params->res_env, env_x);
                rassert(isfinite(mult));

                freq_amp[i] *= mult;
            }
        }
    }
    else
    {
        static const int32_t audio_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
        static const double bandwidth_base = PADSYNTH_DEFAULT_BANDWIDTH_BASE;

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
        for (int32_t i = 0; i < buf_length; ++i)
            freq_phase[i] = Random_get_float_lb(random) * PI2;
    }

    // Set up frequencies in half-complex representation
    float* buf = Sample_get_buffer(entry->sample, 0);

    buf[0] = 0;
    buf[sample_length - 1] = 0;
    buf[sample_length] = 0;

    for (int32_t i = 1; i < buf_length; ++i)
    {
        buf[i * 2 - 1] = (float)(freq_amp[i] * cos(freq_phase[i]));
        buf[i * 2] = (float)(freq_amp[i] * sin(freq_phase[i]));
    }

    // Apply IFFT
    FFT_worker_irfft(fw, buf, sample_length);

    // Normalise
    {
        float max_abs = 0.0f;
        for (int32_t i = 0; i < sample_length; ++i)
            max_abs = max(max_abs, fabsf(buf[i]));

        if (max_abs > 0.0f)
        {
            const float scale = 1.0f / max_abs;
            for (int32_t i = 0; i < sample_length; ++i)
                buf[i] *= scale;
        }
    }

    // Duplicate first frame (for interpolation code)
    buf[sample_length] = buf[0];

    return;
}


static bool apply_padsynth(Proc_padsynth* padsynth, const Padsynth_params* params)
{
    rassert(padsynth != NULL);

    int32_t sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    int sample_count = 1;
    double min_pitch = 0;
    double max_pitch = 0;
    double centre_pitch = 0;
    if (params != NULL)
    {
        sample_length = params->sample_length;
        sample_count = params->sample_count;
        min_pitch = params->min_pitch;
        max_pitch = params->max_pitch;
        centre_pitch = params->centre_pitch;
    }

    // Use only one sample with very small pitch ranges
    if (fabs(min_pitch - max_pitch) < 1)
        sample_count = 1;

    const int32_t buf_length = sample_length / 2;

    double* freq_amp = memory_alloc_items(double, buf_length);
    double* freq_phase = memory_alloc_items(double, buf_length);
    FFT_worker* fw = FFT_worker_init(FFT_WORKER_AUTO, sample_length);
    if (freq_amp == NULL || freq_phase == NULL || fw == NULL)
    {
        memory_free(freq_amp);
        memory_free(freq_phase);
        if (fw != NULL)
            FFT_worker_deinit(fw);
        return false;
    }

    // Allocate new sample map here so that we don't lose old data on allocation failure
    if (padsynth->sample_map == NULL ||
            padsynth->sample_map->sample_length != sample_length ||
            padsynth->sample_map->sample_count != sample_count)
    {
        Padsynth_sample_map* new_sm = new_Padsynth_sample_map(
                sample_count,
                sample_length,
                min_pitch,
                max_pitch,
                centre_pitch);
        if (new_sm == NULL)
        {
            memory_free(freq_amp);
            memory_free(freq_phase);
            FFT_worker_deinit(fw);
            return false;
        }

        del_Padsynth_sample_map(padsynth->sample_map);
        padsynth->sample_map = new_sm;
    }
    else
    {
        Padsynth_sample_map_set_centre_pitch(padsynth->sample_map, centre_pitch);
        Padsynth_sample_map_set_pitch_range(padsynth->sample_map, min_pitch, max_pitch);
    }

    // Build samples
    if (params != NULL)
    {
        AAiter* iter = AAiter_init(AAITER_AUTO, padsynth->sample_map->map);

        int context_index = 0;

        const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(-INFINITY);
        Padsynth_sample_entry* entry = AAiter_get_at_least(iter, key);
        while (entry != NULL)
        {
            make_padsynth_sample(
                    entry,
                    &padsynth->random,
                    context_index,
                    freq_amp,
                    freq_phase,
                    fw,
                    params);
            ++context_index;

            entry = AAiter_get_next(iter);
        }
    }
    else
    {
        const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(-INFINITY);
        Padsynth_sample_entry* entry =
            AAtree_get_at_least(padsynth->sample_map->map, key);
        rassert(entry != NULL);

        make_padsynth_sample(
                entry, &padsynth->random, 0, freq_amp, freq_phase, fw, NULL);
    }

    memory_free(freq_amp);
    memory_free(freq_phase);
    FFT_worker_deinit(fw);

    return true;
}


static void del_Proc_padsynth(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    del_Padsynth_sample_map(padsynth->sample_map);
    memory_free(padsynth);

    return;
}


