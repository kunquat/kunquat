

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_add.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Voice_state_add.h>
#include <devices/param_types/Num_list.h>
#include <devices/param_types/Sample.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Time_env_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>


#define BASE_FUNC_SIZE 4096
#define BASE_FUNC_SIZE_MASK (BASE_FUNC_SIZE - 1)


typedef struct Add_tone
{
    double pitch_factor;
    double volume_factor;
    double panning;
} Add_tone;


typedef enum
{
    MOD_DISABLED = 0,
    MOD_PHASE,
    MOD_LIMIT
} Mod_mode;


typedef struct Proc_add
{
    Device_impl parent;

    Sample* base;
    Sample* mod;
    Mod_mode mod_mode;
    double mod_volume;
    bool mod_env_enabled;
    const Envelope* mod_env;
    bool mod_env_loop_enabled;
    double mod_env_scale_amount;
    double mod_env_center;
    bool force_mod_env_enabled;
    const Envelope* force_mod_env;
    double detune;
    Add_tone tones[HARMONICS_MAX];
    Add_tone mod_tones[HARMONICS_MAX];
} Proc_add;


static bool Proc_add_init(Device_impl* dimpl);

static void Proc_add_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static double sine(double phase, double modifier);

static Set_sample_func      Proc_add_set_base;
static Set_sample_func      Proc_add_set_mod_base;
static Set_int_func         Proc_add_set_mod;
static Set_float_func       Proc_add_set_mod_volume;
static Set_bool_func        Proc_add_set_mod_env_enabled;
static Set_envelope_func    Proc_add_set_mod_env;
static Set_bool_func        Proc_add_set_mod_env_loop_enabled;
static Set_float_func       Proc_add_set_mod_env_scale_amount;
static Set_float_func       Proc_add_set_mod_env_scale_center;
static Set_bool_func        Proc_add_set_force_mod_env_enabled;
static Set_envelope_func    Proc_add_set_force_mod_env;
static Set_float_func       Proc_add_set_tone_pitch;
static Set_float_func       Proc_add_set_tone_volume;
static Set_float_func       Proc_add_set_tone_panning;
static Set_float_func       Proc_add_set_mod_tone_pitch;
static Set_float_func       Proc_add_set_mod_tone_volume;

static Proc_process_vstate_func Proc_add_process_vstate;

static void del_Proc_add(Device_impl* dimpl);


Device_impl* new_Proc_add(Processor* proc)
{
    Proc_add* add = memory_alloc_item(Proc_add);
    if (add == NULL)
        return NULL;

    add->parent.device = (Device*)proc;

    Device_impl_register_init(&add->parent, Proc_add_init);
    Device_impl_register_destroy(&add->parent, del_Proc_add);

    return &add->parent;
}


static bool Proc_add_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    Processor* proc = (Processor*)add->parent.device;
    proc->init_vstate = Proc_add_init_vstate;
    proc->process_vstate = Proc_add_process_vstate;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val) \
    reg_success &= Device_impl_register_set_##type(                       \
            &add->parent, key, def_val, Proc_add_set_##field, NULL)

    REGISTER_SET(sample,    base,           "p_base.wav",               NULL);
    REGISTER_SET(sample,    mod_base,       "p_mod.wav",                NULL);
    REGISTER_SET(int,       mod,            "p_i_mod.json",             MOD_DISABLED);
    REGISTER_SET(float,     mod_volume,     "p_f_mod_volume.json",      0.0);
    REGISTER_SET(bool, mod_env_enabled, "p_b_mod_env_enabled.json",     false);
    REGISTER_SET(envelope,  mod_env,        "p_e_mod_env.json",         NULL);
    REGISTER_SET(bool, mod_env_loop_enabled, "p_b_mod_env_loop_enabled.json", false);
    REGISTER_SET(float, mod_env_scale_amount, "p_f_mod_env_scale_amount.json", 0.0);
    REGISTER_SET(float, mod_env_scale_center, "p_f_mod_env_scale_center.json", 0.0);
    REGISTER_SET(bool, force_mod_env_enabled, "p_b_force_mod_env_enabled.json", false);
    REGISTER_SET(envelope,  force_mod_env,  "p_e_force_mod_env.json",   NULL);
    REGISTER_SET(float,     tone_pitch,     "tone_XX/p_f_pitch.json",   NAN);
    REGISTER_SET(float,     tone_volume,    "tone_XX/p_f_volume.json",  NAN);
    REGISTER_SET(float,     tone_panning,   "tone_XX/p_f_pan.json",     0.0);
    REGISTER_SET(float,     mod_tone_pitch, "mod_XX/p_f_pitch.json",    NAN);
    REGISTER_SET(float,     mod_tone_volume, "mod_XX/p_f_volume.json",  NAN);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    add->base = NULL;
    add->mod = NULL;
    add->mod_mode = MOD_DISABLED;
    add->mod_volume = 1;
    add->mod_env_enabled = false;
    add->mod_env = NULL;
    add->mod_env_loop_enabled = false;
    add->mod_env_scale_amount = 0;
    add->mod_env_center = 440;
    add->force_mod_env_enabled = false;
    add->force_mod_env = NULL;
    add->detune = 1;

    float* buf = memory_alloc_items(float, BASE_FUNC_SIZE);
    float* mod_buf = memory_alloc_items(float, BASE_FUNC_SIZE);
    if (buf == NULL || mod_buf == NULL)
    {
        memory_free(buf);
        memory_free(mod_buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->base = new_Sample_from_buffers(&buf, 1, BASE_FUNC_SIZE);
    if (add->base == NULL)
    {
        memory_free(buf);
        memory_free(mod_buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->mod = new_Sample_from_buffers(&mod_buf, 1, BASE_FUNC_SIZE);
    if (add->mod == NULL)
    {
        memory_free(mod_buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    for (int i = 0; i < BASE_FUNC_SIZE; ++i)
    {
        buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
        mod_buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
    }

    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
        add->tones[h].panning = 0;

        add->mod_tones[h].pitch_factor = 0;
        add->mod_tones[h].volume_factor = 0;
        add->mod_tones[h].panning = 0;
    }

    add->tones[0].pitch_factor = 1.0;
    add->tones[0].volume_factor = 1.0;

    add->mod_tones[0].pitch_factor = 1.0;
    add->mod_tones[0].volume_factor = 1.0;

    return true;
}


const char* Proc_add_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_add));

        return size_str;
    }

    return NULL;
}


static void Proc_add_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(vstate != NULL);

    Proc_add* add = (Proc_add*)proc->parent.dimpl;
    Voice_state_add* add_state = (Voice_state_add*)vstate;

    add_state->tone_limit = 0;
    add_state->mod_tone_limit = 0;

    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        if (add->tones[h].pitch_factor <= 0 ||
                add->tones[h].volume_factor <= 0)
            continue;

        add_state->tone_limit = h + 1;
        add_state->tones[h].phase = 0;
    }

    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        if (add->mod_tones[h].pitch_factor <= 0 ||
                add->mod_tones[h].volume_factor <= 0)
            continue;

        add_state->mod_tone_limit = h + 1;
        add_state->mod_tones[h].phase = 0;
    }

    add_state->mod_active = add->mod_mode != MOD_DISABLED;

    Time_env_state_init(&add_state->mod_env_state);

    return;
}


static uint32_t Proc_add_process_vstate(
        const Processor* proc,
        Proc_state* proc_state,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    Proc_add* add = (Proc_add*)proc->parent.dimpl;
    Voice_state_add* add_state = (Voice_state_add*)vstate;
    assert(is_p2(BASE_FUNC_SIZE));

    const float* actual_pitches = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const float* actual_forces = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    float* audio_l = Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L);
    float* audio_r = Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R);

    static const int ADD_WORK_BUFFER_MOD = WORK_BUFFER_IMPL_1;

    const double inv_audio_rate = 1.0 / audio_rate;

    float* mod_values = Work_buffers_get_buffer_contents_mut(wbs, ADD_WORK_BUFFER_MOD);

    // Get modulation
    for (int32_t i = buf_start; i < buf_stop; ++i)
        mod_values[i] = 0;

    if (add_state->mod_active)
    {
        const float* mod_base = Sample_get_buffer(add->mod, 0);
        assert(mod_base != NULL);

        // Add modulation tones
        const double mod_volume = add->mod_volume;

        for (int h = 0; h < add_state->mod_tone_limit; ++h)
        {
            Add_tone* mod_tone = &add->mod_tones[h];
            const double pitch_factor = mod_tone->pitch_factor;
            const double volume_factor = mod_tone->volume_factor;

            if ((pitch_factor <= 0) || (volume_factor <= 0))
                continue;

            const double pitch_factor_inv_audio_rate = pitch_factor * inv_audio_rate;

            Add_tone_state* mod_tone_state = &add_state->mod_tones[h];
            double phase = mod_tone_state->phase;

            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const float actual_pitch = actual_pitches[i];

                float mod_value = mod_values[i];

                const double pos = phase * BASE_FUNC_SIZE;
                const uint32_t pos1 = (uint32_t)pos & BASE_FUNC_SIZE_MASK;
                const uint32_t pos2 = (pos1 + 1) & BASE_FUNC_SIZE_MASK;
                const float item1 = mod_base[pos1];
                const float item_diff = mod_base[pos2] - item1;
                const double lerp_val = pos - floor(pos);
                mod_value +=
                    (item1 + (lerp_val * item_diff)) * volume_factor * mod_volume;

                phase += actual_pitch * pitch_factor_inv_audio_rate;
                if (phase >= 1)
                    phase -= floor(phase);

                mod_values[i] = mod_value;
            }

            mod_tone_state->phase = phase;
        }

        // Apply force->mod envelope
        const Envelope* force_mod_env = add->force_mod_env;
        if (add->force_mod_env_enabled && (force_mod_env != NULL))
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const float actual_force = actual_forces[i];

                const double force = min(1, actual_force);
                const double factor = Envelope_get_value(force_mod_env, force);
                assert(isfinite(factor));
                mod_values[i] *= factor;
            }
        }

        // Apply mod envelope
        if (add->mod_env_enabled && (add->mod_env != NULL))
        {
            const int32_t mod_env_stop = Time_env_state_process(
                    &add_state->mod_env_state,
                    add->mod_env,
                    add->mod_env_loop_enabled,
                    add->mod_env_scale_amount,
                    add->mod_env_center,
                    0, // sustain
                    0, 1, // range
                    wbs,
                    buf_start,
                    buf_stop,
                    audio_rate);

            float* time_env = Work_buffers_get_buffer_contents_mut(
                    wbs, WORK_BUFFER_TIME_ENV);

            // Check the end of envelope processing
            if (add_state->mod_env_state.is_finished)
            {
                const double* last_node = Envelope_get_node(
                        add->mod_env, Envelope_node_count(add->mod_env) - 1);
                const double last_value = last_node[1];
                if (last_value == 0)
                    add_state->mod_active = false;

                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = mod_env_stop; i < buf_stop; ++i)
                    time_env[i] = last_value;
            }

            for (int32_t i = buf_start; i < buf_stop; ++i)
                mod_values[i] *= time_env[i];
        }
    }

    // Add base waveform tones
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        audio_l[i] = 0;
        audio_r[i] = 0;
    }

    const float* base = Sample_get_buffer(add->base, 0);

    for (int h = 0; h < add_state->tone_limit; ++h)
    {
        Add_tone* tone = &add->tones[h];
        const double pitch_factor = tone->pitch_factor;
        const double volume_factor = tone->volume_factor;

        if ((pitch_factor <= 0) || (volume_factor <= 0))
            continue;

        const double panning = tone->panning;
        const double pitch_factor_inv_audio_rate = pitch_factor * inv_audio_rate;

        Add_tone_state* tone_state = &add_state->tones[h];
        double phase = tone_state->phase;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float actual_pitch = actual_pitches[i];
            const float mod_val = mod_values[i];

            // Note: + mod_val is specific to phase modulation
            const double actual_phase = phase + mod_val;
            const double pos = actual_phase * BASE_FUNC_SIZE;

            // Note: direct cast of negative doubles to uint32_t is undefined
            const uint32_t pos1 = (uint32_t)(int32_t)pos & BASE_FUNC_SIZE_MASK;
            const uint32_t pos2 = (pos1 + 1) & BASE_FUNC_SIZE_MASK;

            const float item1 = base[pos1];
            const float item_diff = base[pos2] - item1;
            const double lerp_val = pos - floor(pos);
            const double value = (item1 + (lerp_val * item_diff)) * volume_factor;

            audio_l[i] += value * (1 - panning);
            audio_r[i] += value * (1 + panning);

            phase += actual_pitch * pitch_factor_inv_audio_rate;
            if (phase >= 1)
                phase -= floor(phase);
        }

        tone_state->phase = phase;
    }

    // Apply actual force
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_force = actual_forces[i];
        audio_l[i] *= actual_force;
        audio_r[i] *= actual_force;
    }

    Proc_ramp_attack(proc, vstate, wbs, 2, audio_rate, buf_start, buf_stop);

    vstate->pos = 1; // XXX: hackish

    return buf_stop;
}


static double sine(double phase, double modifier)
{
    ignore(modifier);
    return -sin(phase * PI * 2);
}


static void fill_buf(float* buf, const Sample* sample)
{
    assert(buf != NULL);

    if ((sample != NULL) && (sample->data[0] != NULL) && sample->is_float)
    {
        int32_t available = min(sample->len, BASE_FUNC_SIZE);

        const float* from_buf = sample->data[0];

        for (int i = 0; i < available; ++i)
            buf[i] = clamp(from_buf[i], -1.0, 1.0);
        for (int i = available; i < BASE_FUNC_SIZE; ++i)
            buf[i] = 0;
    }
    else
    {
        for (int i = 0; i < BASE_FUNC_SIZE; ++i)
            buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
    }

    return;
}


static bool Proc_add_set_base(
        Device_impl* dimpl, Key_indices indices, const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    fill_buf(Sample_get_buffer(add->base, 0), value);

    return true;
}


static bool Proc_add_set_mod_base(
        Device_impl* dimpl, Key_indices indices, const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    fill_buf(Sample_get_buffer(add->mod, 0), value);

    return true;
}


static bool Proc_add_set_mod(
        Device_impl* dimpl, Key_indices indices, int64_t value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    if (value >= MOD_DISABLED && value < MOD_LIMIT)
        add->mod_mode = value;
    else
        add->mod_mode = MOD_DISABLED;

    return true;
}


static bool Proc_add_set_mod_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    if (isfinite(value))
        add->mod_volume = exp2(value / 6.0);
    else
        add->mod_volume = 1.0;

    return true;
}


static bool Proc_add_set_mod_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->mod_env_enabled = enabled;

    return true;
}


static bool Proc_add_set_mod_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    bool valid = true;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        double* node = Envelope_get_node(value, 0);
        if (node[0] != 0)
            valid = false;

        node = Envelope_get_node(
                value,
                Envelope_node_count(value) - 1);

        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            node = Envelope_get_node(value, i);
            if (node[1] < 0)
            {
                valid = false;
                break;
            }
        }
    }
    else
    {
        valid = false;
    }

    add->mod_env = valid ? value : NULL;

    return true;
}


static bool Proc_add_set_mod_env_loop_enabled(
        Device_impl* dimpl, Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->mod_env_loop_enabled = enabled;

    return true;
}


static bool Proc_add_set_mod_env_scale_amount(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    add->mod_env_scale_amount = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_add_set_mod_env_scale_center(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    add->mod_env_center = isfinite(value) ? exp2(value / 1200) * 440 : 440;

    return true;
}


static bool Proc_add_set_force_mod_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->force_mod_env_enabled = value;

    return true;
}


static bool Proc_add_set_force_mod_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    bool valid = true;

    if (value != NULL && Envelope_node_count(value) > 1)
    {
        double* node = Envelope_get_node(value, 0);
        if (node[0] != 0)
            valid = false;

        node = Envelope_get_node(value, Envelope_node_count(value) - 1);
        if (node[0] != 1)
            valid = false;

        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            node = Envelope_get_node(value, i);
            if (node[1] < 0)
            {
                valid = false;
                break;
            }
        }
    }
    else
    {
        valid = false;
    }

    add->force_mod_env = valid ? value : NULL;

    return true;
}


static bool Proc_add_set_tone_pitch(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->tones[ti].pitch_factor = value;
    else
        add->tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (isfinite(value))
        add->tones[ti].volume_factor = exp2(value / 6.0);
    else
        add->tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_panning(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value >= -1.0 && value <= 1.0)
        add->tones[ti].panning = value;
    else
        add->tones[ti].panning = 0.0;

    return true;
}


static bool Proc_add_set_mod_tone_pitch(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->mod_tones[ti].pitch_factor = value;
    else
        add->mod_tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_mod_tone_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (isfinite(value))
        add->mod_tones[ti].volume_factor = exp2(value / 6.0);
    else
        add->mod_tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static void del_Proc_add(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_add* add = (Proc_add*)dimpl;
    del_Sample(add->base);
    del_Sample(add->mod);
    memory_free(add);

    return;
}


