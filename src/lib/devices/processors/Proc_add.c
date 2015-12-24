

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


#include <devices/processors/Proc_add.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Voice_state_add.h>
#include <devices/param_types/Sample.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BASE_FUNC_SIZE 4096
#define BASE_FUNC_SIZE_MASK (BASE_FUNC_SIZE - 1)


typedef struct Add_tone
{
    double pitch_factor;
    double volume_factor;
    double panning;
} Add_tone;


typedef struct Proc_add
{
    Device_impl parent;

    Sample* base;
    bool is_ramp_attack_enabled;
    Add_tone tones[HARMONICS_MAX];
} Proc_add;


static bool Proc_add_init(Device_impl* dimpl);

static void Proc_add_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static double sine(double phase, double modifier);

static Set_sample_func  Proc_add_set_base;
static Set_bool_func    Proc_add_set_ramp_attack;
static Set_float_func   Proc_add_set_tone_pitch;
static Set_float_func   Proc_add_set_tone_volume;
static Set_float_func   Proc_add_set_tone_panning;

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
    REGISTER_SET(bool,      ramp_attack,    "p_b_ramp_attack.json",     true);
    REGISTER_SET(float,     tone_pitch,     "tone_XX/p_f_pitch.json",   NAN);
    REGISTER_SET(float,     tone_volume,    "tone_XX/p_f_volume.json",  NAN);
    REGISTER_SET(float,     tone_panning,   "tone_XX/p_f_pan.json",     0.0);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    add->base = NULL;
    add->is_ramp_attack_enabled = true;

    float* buf = memory_alloc_items(float, BASE_FUNC_SIZE + 1);
    if (buf == NULL)
    {
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->base = new_Sample_from_buffers(&buf, 1, BASE_FUNC_SIZE + 1);
    if (add->base == NULL)
    {
        memory_free(buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    for (int i = 0; i < BASE_FUNC_SIZE; ++i)
        buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
    buf[BASE_FUNC_SIZE] = buf[0];

    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
        add->tones[h].panning = 0;
    }

    add->tones[0].pitch_factor = 1.0;
    add->tones[0].volume_factor = 1.0;

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

    for (int h = 0; h < HARMONICS_MAX; ++h)
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
    const double inv_audio_rate = 1.0 / audio_rate;

    const float* base = Sample_get_buffer(add->base, 0);

    for (int h = 0; h < add_state->tone_limit; ++h)
    {
        Add_tone* tone = &add->tones[h];
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
                const double pos = actual_phase * BASE_FUNC_SIZE;

                // Note: direct cast of negative doubles to uint32_t is undefined
                const uint32_t pos1 = (uint32_t)(int32_t)pos & BASE_FUNC_SIZE_MASK;
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
        Proc_ramp_attack(proc, vstate, out_buffer, 2, audio_rate, buf_start, buf_stop);

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

    buf[BASE_FUNC_SIZE] = buf[0];

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


static bool Proc_add_set_ramp_attack(
        Device_impl* dimpl, Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->is_ramp_attack_enabled = enabled;

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


static void del_Proc_add(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_add* add = (Proc_add*)dimpl;
    del_Sample(add->base);
    memory_free(add);

    return;
}


