

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


#include <init/devices/processors/Proc_padsynth.h>

#include <containers/AAtree.h>
#include <containers/Array.h>
#include <debug/assert.h>
#include <init/Background_loader.h>
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
static Set_float_func           Proc_padsynth_set_start_pos;
static Set_bool_func            Proc_padsynth_set_start_pos_var_enabled;
static Set_bool_func            Proc_padsynth_set_round_start_pos_var_to_period;
static Set_float_func           Proc_padsynth_set_start_pos_var;

static bool apply_padsynth(
        Proc_padsynth* padsynth,
        const Padsynth_params* params,
        Background_loader* bkg_loader);

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
        int sample_count, int32_t sample_length)
{
    rassert(sample_count > 0);
    rassert(sample_count <= 128);
    rassert(sample_length >= PADSYNTH_MIN_SAMPLE_LENGTH);
    rassert(sample_length <= PADSYNTH_MAX_SAMPLE_LENGTH);
    rassert(is_p2(sample_length));

    Padsynth_sample_map* sm = memory_alloc_item(Padsynth_sample_map);
    if (sm == NULL)
        return NULL;

    sm->sample_count = sample_count;
    sm->sample_length = sample_length;
    sm->min_pitch = NAN;
    sm->max_pitch = NAN;
    sm->centre_pitch = NAN;
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

        entry->centre_pitch = (double)i; // placeholder to get unique keys :-P

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

    sm->min_pitch = min_pitch;
    sm->max_pitch = max_pitch;

    return;
}


static double round_to_period(double cents, int32_t sample_length)
{
    rassert(isfinite(cents));
    rassert(sample_length > 0);

    const double entry_Hz = cents_to_Hz(cents);
    const double cycle_length = PADSYNTH_DEFAULT_AUDIO_RATE / entry_Hz;
    const double cycle_count = sample_length / cycle_length;
    const double rounded_cycle_count = round(cycle_count);
    const double rounded_cycle_length = sample_length / rounded_cycle_count;
    const double rounded_entry_Hz = PADSYNTH_DEFAULT_AUDIO_RATE / rounded_cycle_length;
    const double rounded_entry_cents = log2(rounded_entry_Hz / 440) * 1200.0;

    return rounded_entry_cents;
}


static void Padsynth_sample_map_update_sample_pitches(
        Padsynth_sample_map* sm, bool enable_rounding)
{
    rassert(sm != NULL);
    rassert(isfinite(sm->min_pitch));
    rassert(isfinite(sm->max_pitch));
    rassert(isfinite(sm->centre_pitch));

    AAiter* iter = AAiter_init(AAITER_AUTO, sm->map);

    const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(-INFINITY);
    Padsynth_sample_entry* entry = AAiter_get_at_least(iter, key);
    if (sm->sample_count == 1)
    {
        rassert(entry != NULL);
        const double unrounded_pitch = (sm->min_pitch + sm->max_pitch) * 0.5;
        if (enable_rounding)
            entry->centre_pitch = round_to_period(unrounded_pitch, sm->sample_length);
        else
            entry->centre_pitch = unrounded_pitch;

        entry = AAiter_get_next(iter);
    }
    else
    {
        for (int i = 0; i < sm->sample_count; ++i)
        {
            rassert(entry != NULL);
            const double unrounded_pitch =
                lerp(sm->min_pitch, sm->max_pitch, i / (double)(sm->sample_count - 1));
            if (enable_rounding)
                entry->centre_pitch = round_to_period(unrounded_pitch, sm->sample_length);
            else
                entry->centre_pitch = unrounded_pitch;

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

    padsynth->start_pos = 0.0;
    padsynth->is_start_pos_var_enabled = true;
    padsynth->round_start_pos_var_to_period = false;
    padsynth->start_pos_var = 1.0;

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
                padsynth, bool, stereo, "p_b_stereo.json", false) &&
            REGISTER_SET_FIXED_STATE(
                padsynth, float, start_pos, "p_f_start_pos.json", 0.0) &&
            REGISTER_SET_FIXED_STATE(
                padsynth,
                bool,
                start_pos_var_enabled,
                "p_b_start_pos_var_enabled.json",
                true) &&
            REGISTER_SET_FIXED_STATE(
                padsynth,
                bool,
                round_start_pos_var_to_period,
                "p_b_round_start_pos_var_to_period.json",
                false) &&
            REGISTER_SET_FIXED_STATE(
                padsynth, float, start_pos_var, "p_f_start_pos_var.json", 1.0)
        ))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    Random_init(&padsynth->random, "PADsynth");

    if (!apply_padsynth(padsynth, NULL, NULL))
    {
        del_Device_impl(&padsynth->parent);
        return NULL;
    }

    return &padsynth->parent;
}


static bool Proc_padsynth_set_params(
        Device_impl* dimpl,
        const Key_indices indices,
        const Padsynth_params* params,
        Background_loader* bkg_loader)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;

    return apply_padsynth(padsynth, params, bkg_loader);
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


static bool Proc_padsynth_set_start_pos_var_enabled(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->is_start_pos_var_enabled = enabled;

    return true;
}


static bool Proc_padsynth_set_round_start_pos_var_to_period(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->round_start_pos_var_to_period = enabled;

    return true;
}


static bool Proc_padsynth_set_start_pos_var(
        Device_impl* dimpl, const Key_indices indices, double var)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    const double applied_var = ((0 <= var) && (var <= 1.0)) ? var : 0.0;

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->start_pos_var = applied_var;

    return true;
}


static bool Proc_padsynth_set_start_pos(
        Device_impl* dimpl, const Key_indices indices, double pos)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    const double applied_pos = ((0 <= pos) && (pos <= 1.0)) ? pos : 0.0;

    Proc_padsynth* padsynth = (Proc_padsynth*)dimpl;
    padsynth->start_pos = applied_pos;

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


static double get_phase_spread(double freq_i, double bandwidth_i)
{
    const double x = freq_i / bandwidth_i;
    return 1.0 - exp(-x * x);
}


typedef struct Callback_data
{
    Padsynth_sample_entry* entry;
    int context_index;
    double* freq_amp;
    double* freq_phase;
    FFT_worker fw;
    const Padsynth_params* params;
} Callback_data;


static void del_Callback_data(Callback_data* cb_data)
{
    if (cb_data == NULL)
        return;

    memory_free(cb_data->freq_amp);
    memory_free(cb_data->freq_phase);
    FFT_worker_deinit(&cb_data->fw);

    memory_free(cb_data);

    return;
}


static Callback_data* new_Callback_data(const Padsynth_params* params)
{
    Callback_data* cb_data = memory_alloc_item(Callback_data);
    if (cb_data == NULL)
        return NULL;

    cb_data->entry = NULL;
    cb_data->context_index = 0;
    cb_data->freq_amp = NULL;
    cb_data->freq_phase = NULL;
    cb_data->params = params;

    int32_t sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    if (params != NULL)
        sample_length = params->sample_length;

    const int32_t buf_length = sample_length / 2;

    cb_data->freq_amp = memory_alloc_items(double, buf_length);
    cb_data->freq_phase = memory_alloc_items(double, buf_length);
    FFT_worker* fw = FFT_worker_init(&cb_data->fw, sample_length);
    if (cb_data->freq_amp == NULL || cb_data->freq_phase == NULL || fw == NULL)
    {
        del_Callback_data(cb_data);
        return NULL;
    }

    return cb_data;
}


static void cleanup_cb_data(Error* error, void* user_data)
{
    rassert(error != NULL);
    rassert(user_data != NULL);

    Callback_data* cb_data = user_data;
    del_Callback_data(cb_data);

    return;
}


static void make_padsynth_sample(Error* error, void* user_data)
{
    rassert(error != NULL);
    rassert(user_data != NULL);

    Callback_data* cb_data = user_data;
    rassert(cb_data->entry != NULL);
    rassert(cb_data->context_index >= 0);
    rassert(cb_data->context_index < PADSYNTH_MAX_SAMPLE_COUNT);
    rassert(cb_data->freq_amp != NULL);
    rassert(cb_data->freq_phase != NULL);

    char context_str[16] = "";
    snprintf(context_str, 16, "PADsynth%hd", (short)cb_data->context_index);
    Random* random = Random_init(RANDOM_AUTO, context_str);

    double* freq_amp = cb_data->freq_amp;
    double* freq_phase = cb_data->freq_phase;
    const Padsynth_params* params = cb_data->params;

    int32_t sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    if (params != NULL)
        sample_length = params->sample_length;

    const int32_t buf_length = sample_length / 2;

    for (int32_t i = 0; i < buf_length; ++i)
    {
        freq_amp[i] = 0;
        freq_phase[i] = 0;
    }

    const bool use_phase_data = (params != NULL) ? params->use_phase_data : false;

    if (params != NULL)
    {
        const int32_t audio_rate = params->audio_rate;
        const double bandwidth_base = params->bandwidth_base;
        const double bandwidth_scale = params->bandwidth_scale;

        const double freq = cents_to_Hz(cb_data->entry->centre_pitch);

        const double ps_bw_base = params->phase_spread_bandwidth_base;
        const double ps_bw_scale = params->phase_spread_bandwidth_scale;
        const double phase_var_at_h = params->phase_var_at_harmonic;
        const double phase_var_off_h = params->phase_var_off_harmonic;

        const int32_t nyquist = params->audio_rate / 2;

        char phase_context_str[16] = "";
        snprintf(phase_context_str, 16, "PADphase%hd", (short)cb_data->context_index);
        Random* phase_random = Random_init(RANDOM_AUTO, phase_context_str);

        for (int64_t h = 0; h < Array_get_size(params->harmonics); ++h)
        {
            const Padsynth_harmonic* harmonic = Array_get_ref(params->harmonics, h);

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

            if (use_phase_data)
            {
                // Phase spread
                const double ps_bandwidth_Hz =
                    (exp2(ps_bw_base / 1200.0) - 1.0) *
                    freq *
                    pow(harmonic->freq_mul, ps_bw_scale);
                const double ps_bandwidth_i = ps_bandwidth_Hz / (2.0 * audio_rate);

                Random_set_seed(phase_random, Random_get_uint64(random));

                for (int32_t i = buf_start; i < buf_stop; ++i)
                {
                    // Add amplitude and phase of the harmonic
                    const double orig_amp = freq_amp[i];
                    const double orig_phase = freq_phase[i];
                    const double orig_real = orig_amp * cos(orig_phase);
                    const double orig_imag = orig_amp * sin(orig_phase);

                    const double harmonic_profile =
                        profile((i / (double)sample_length) - freq_i, bandwidth_i);

                    const double add_amp = harmonic_profile * harmonic->amplitude;
                    const double add_real = add_amp * cos(harmonic->phase);
                    const double add_imag = add_amp * sin(harmonic->phase);

                    const double new_real = orig_real + add_real;
                    const double new_imag = orig_imag + add_imag;
                    const double new_amp =
                        sqrt((new_real * new_real) + (new_imag * new_imag));
                    double new_phase = atan2(new_imag, new_real);
                    if (new_phase < 0)
                        new_phase += PI2;

                    // Add phase variation
                    const double phase_spread_norm = get_phase_spread(
                            (i / (double)sample_length) - freq_i, ps_bandwidth_i);
                    const double phase_spread =
                        lerp(phase_var_at_h, phase_var_off_h, phase_spread_norm);
                    new_phase += Random_get_float_lb(phase_random) * PI2 * phase_spread;
                    if (new_phase >= PI2)
                        new_phase = fmod(new_phase, PI2);

                    freq_amp[i] = new_amp;
                    freq_phase[i] = new_phase;
                }
            }
            else
            {
                for (int32_t i = buf_start; i < buf_stop; ++i)
                {
                    const double harmonic_profile =
                        profile((i / (double)sample_length) - freq_i, bandwidth_i);
                    freq_amp[i] += harmonic_profile * harmonic->amplitude;
                }
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

    if (!use_phase_data)
    {
        // Add randomised phases
        for (int32_t i = 0; i < buf_length; ++i)
            freq_phase[i] = Random_get_float_lb(random) * PI2;
    }

    // Set up frequencies in half-complex representation
    float* buf = Sample_get_buffer(cb_data->entry->sample, 0);
    rassert(buf != NULL);

    buf[0] = 0;
    buf[sample_length - 1] = 0;
    buf[sample_length] = 0;

    for (int32_t i = 1; i < buf_length; ++i)
    {
        buf[i * 2 - 1] = (float)(freq_amp[i] * cos(freq_phase[i]));
        buf[i * 2] = (float)(freq_amp[i] * sin(freq_phase[i]));
    }

    // Apply IFFT
    FFT_worker_irfft(&cb_data->fw, buf, sample_length);

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


static bool apply_padsynth(
        Proc_padsynth* padsynth,
        const Padsynth_params* params,
        Background_loader* bkg_loader)
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
    // TODO: Make sure we don't use more samples than we can differentiate
    //       with rounded pitches
    if (fabs(min_pitch - max_pitch) < 1)
        sample_count = 1;

    Callback_data* cb_datas[PADSYNTH_MAX_SAMPLE_COUNT] = { NULL };

    int cb_data_count = 0;
    for (int i = 0; i < sample_count; ++i)
    {
        cb_datas[i] = new_Callback_data(params);
        if (cb_datas[i] == NULL)
            break;

        cb_data_count = i + 1;
    }

    if (cb_data_count == 0)
        return false;

    bool last_cb_data_is_direct = (cb_data_count < sample_count);

    // Allocate new sample map here so that we don't lose old data on allocation failure
    if (padsynth->sample_map == NULL ||
            padsynth->sample_map->sample_length != sample_length ||
            padsynth->sample_map->sample_count != sample_count)
    {
        Padsynth_sample_map* new_sm =
            new_Padsynth_sample_map(sample_count, sample_length);
        if (new_sm == NULL)
        {
            for (int i = 0; i < cb_data_count; ++i)
                del_Callback_data(cb_datas[i]);
            return false;
        }

        del_Padsynth_sample_map(padsynth->sample_map);
        padsynth->sample_map = new_sm;
    }

    Padsynth_sample_map_set_centre_pitch(padsynth->sample_map, centre_pitch);
    Padsynth_sample_map_set_pitch_range(padsynth->sample_map, min_pitch, max_pitch);
    const bool enable_rounding = (params != NULL) ? params->round_to_period : true;
    Padsynth_sample_map_update_sample_pitches(padsynth->sample_map, enable_rounding);

    // Build samples
    if (params != NULL)
    {
        rassert(bkg_loader != NULL);

        AAiter* iter = AAiter_init(AAITER_AUTO, padsynth->sample_map->map);

        int context_index = 0;

        const Padsynth_sample_entry* key = PADSYNTH_SAMPLE_ENTRY_KEY(-INFINITY);
        Padsynth_sample_entry* entry = AAiter_get_at_least(iter, key);
        while (entry != NULL)
        {
            Callback_data* cb_data = cb_datas[min(context_index, cb_data_count - 1)];
            rassert(cb_data != NULL);

            cb_data->entry = entry;
            cb_data->context_index = context_index;

            if (last_cb_data_is_direct && (context_index >= cb_data_count - 1))
            {
                Error* error = ERROR_AUTO;
                make_padsynth_sample(error, cb_data);
                rassert(!Error_is_set(error));
            }
            else
            {
                Background_loader_task* task = MAKE_BACKGROUND_LOADER_TASK(
                        make_padsynth_sample, cleanup_cb_data, cb_data);

                if (!Background_loader_add_task(bkg_loader, task))
                {
                    Error* error = ERROR_AUTO;
                    make_padsynth_sample(error, cb_data);
                    cleanup_cb_data(error, cb_data);
                    rassert(!Error_is_set(error));
                }
            }

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

        Callback_data* cb_data = cb_datas[0];
        rassert(cb_data != NULL);

        cb_data->entry = entry;
        cb_data->context_index = 0;

        rassert(!last_cb_data_is_direct);

        Background_loader_task* task = MAKE_BACKGROUND_LOADER_TASK(
                make_padsynth_sample, cleanup_cb_data, cb_data);
        if ((bkg_loader == NULL) || !Background_loader_add_task(bkg_loader, task))
        {
            Error* error = ERROR_AUTO;
            make_padsynth_sample(error, cb_data);
            cleanup_cb_data(error, cb_data);
            rassert(!Error_is_set(error));
        }
    }

    if (last_cb_data_is_direct)
        del_Callback_data(cb_datas[cb_data_count - 1]);

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


