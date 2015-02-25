

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
#include <devices/Generator.h>
#include <devices/generators/Generator_add.h>
#include <devices/generators/Generator_common.h>
#include <devices/generators/Voice_state_add.h>
#include <devices/param_types/Num_list.h>
#include <devices/param_types/Sample.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>


#define BASE_FUNC_SIZE 4096


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


typedef struct Generator_add
{
    Device_impl parent;

    Sample* base;
    Sample* mod;
    Mod_mode mod_mode;
    double mod_volume;
    bool mod_env_enabled;
    const Envelope* mod_env;
    double mod_env_scale_amount;
    double mod_env_center;
    bool force_mod_env_enabled;
    const Envelope* force_mod_env;
    double detune;
    Add_tone tones[HARMONICS_MAX];
    Add_tone mod_tones[HARMONICS_MAX];
} Generator_add;


static bool Generator_add_init(Device_impl* dimpl);

static void Generator_add_init_vstate(
        const Generator* gen, const Gen_state* gen_state, Voice_state* vstate);

static double sine(double phase, double modifier);

static Set_sample_func      Generator_add_set_base;
static Set_sample_func      Generator_add_set_mod_base;
static Set_int_func         Generator_add_set_mod;
static Set_float_func       Generator_add_set_mod_volume;
static Set_bool_func        Generator_add_set_mod_env_enabled;
static Set_envelope_func    Generator_add_set_mod_env;
static Set_float_func       Generator_add_set_mod_env_scale_amount;
static Set_float_func       Generator_add_set_mod_env_scale_center;
static Set_bool_func        Generator_add_set_force_mod_env_enabled;
static Set_envelope_func    Generator_add_set_force_mod_env;
static Set_float_func       Generator_add_set_tone_pitch;
static Set_float_func       Generator_add_set_tone_volume;
static Set_float_func       Generator_add_set_tone_panning;
static Set_float_func       Generator_add_set_mod_tone_pitch;
static Set_float_func       Generator_add_set_mod_tone_volume;

static Generator_mix_func Generator_add_mix;

static void del_Generator_add(Device_impl* gen_impl);


Device_impl* new_Generator_add(Generator* gen)
{
    Generator_add* add = memory_alloc_item(Generator_add);
    if (add == NULL)
        return NULL;

    add->parent.device = (Device*)gen;

    Device_impl_register_init(&add->parent, Generator_add_init);
    Device_impl_register_destroy(&add->parent, del_Generator_add);

    return &add->parent;
}


static bool Generator_add_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Generator_add* add = (Generator_add*)dimpl;

    Generator* gen = (Generator*)add->parent.device;
    gen->init_vstate = Generator_add_init_vstate;
    gen->mix = Generator_add_mix;

    bool reg_success = true;

    reg_success &= Device_impl_register_set_sample(
            &add->parent, "p_base.wav", NULL, Generator_add_set_base, NULL);
    reg_success &= Device_impl_register_set_sample(
            &add->parent, "p_mod.wav", NULL, Generator_add_set_mod_base, NULL);
    reg_success &= Device_impl_register_set_int(
            &add->parent,
            "p_i_mod.json",
            MOD_DISABLED,
            Generator_add_set_mod,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "p_f_mod_volume.json",
            0.0,
            Generator_add_set_mod_volume,
            NULL);
    reg_success &= Device_impl_register_set_bool(
            &add->parent,
            "p_b_mod_env_enabled.json",
            false,
            Generator_add_set_mod_env_enabled,
            NULL);
    reg_success &= Device_impl_register_set_envelope(
            &add->parent,
            "p_e_mod_env.json",
            NULL,
            Generator_add_set_mod_env,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "p_f_mod_env_scale_amount.json",
            0.0,
            Generator_add_set_mod_env_scale_amount,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "p_f_mod_env_scale_center.json",
            0.0,
            Generator_add_set_mod_env_scale_center,
            NULL);
    reg_success &= Device_impl_register_set_bool(
            &add->parent,
            "p_b_force_mod_env_enabled.json",
            NULL,
            Generator_add_set_force_mod_env_enabled,
            NULL);
    reg_success &= Device_impl_register_set_envelope(
            &add->parent,
            "p_e_force_mod_env.json",
            NULL,
            Generator_add_set_force_mod_env,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "tone_XX/p_f_pitch.json",
            NAN,
            Generator_add_set_tone_pitch,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "tone_XX/p_f_volume.json",
            NAN,
            Generator_add_set_tone_volume,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "tone_XX/p_f_pan.json",
            0.0,
            Generator_add_set_tone_panning,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "mod_XX/p_f_pitch.json",
            NAN,
            Generator_add_set_mod_tone_pitch,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &add->parent,
            "mod_XX/p_f_volume.json",
            NAN,
            Generator_add_set_mod_tone_volume,
            NULL);

    if (!reg_success)
        return false;

    add->base = NULL;
    add->mod = NULL;
    add->mod_mode = MOD_DISABLED;
    add->mod_volume = 1;
    add->mod_env_enabled = false;
    add->mod_env = NULL;
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

#if 0
    Sample_set_loop_start(add->base, 0);
    Sample_set_loop_end(add->base, BASE_FUNC_SIZE);
    Sample_set_loop(add->base, SAMPLE_LOOP_UNI);
#endif

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


const char* Generator_add_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "add"));
    assert(property_type != NULL);
    (void)gen;

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_add));

        return size_str;
    }

    return NULL;
}


static void Generator_add_init_vstate(
        const Generator* gen, const Gen_state* gen_state, Voice_state* vstate)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "add"));
    assert(gen_state != NULL);
    (void)gen_state;
    assert(vstate != NULL);

    Generator_add* add = (Generator_add*)gen->parent.dimpl;
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
    add_state->mod_env_pos = 0;
    add_state->mod_env_next_node = 0;
    add_state->mod_env_value = NAN;
    add_state->mod_env_update = 0;
    add_state->mod_env_scale = NAN;

    return;
}


static uint32_t Generator_add_mix(
        const Generator* gen,
        Gen_state* gen_state,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "add"));
    assert(gen_state != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Generator_add* add = (Generator_add*)gen->parent.dimpl;
    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen_state, vstate, offset, bufs);
    Voice_state_add* add_state = (Voice_state_add*)vstate;
    assert(is_p2(BASE_FUNC_SIZE));

    Generator_common_handle_pitch(gen, vstate, wbs, nframes, offset);

    const int32_t force_extent = Generator_common_handle_force(
            gen, ins_state, vstate, wbs, freq, nframes, offset);

    const bool force_ended = (force_extent < (int32_t)nframes);
    if (force_ended)
        nframes = force_extent;

    const Work_buffer* wb_actual_pitches = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    const float* actual_pitches = Work_buffer_get_contents(wb_actual_pitches) + 1;
    const float* actual_forces = Work_buffer_get_contents(wb_actual_forces) + 1;

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
    float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

    uint32_t mixed = offset;
    for (; mixed < nframes && vstate->active; ++mixed)
    {
        //Generator_common_handle_pitch(gen, vstate);

        const float actual_pitch = actual_pitches[mixed];
        const float prev_actual_pitch = actual_pitches[(int32_t)mixed - 1];
        const float actual_force = actual_forces[mixed];

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = 0;
        double mod_val = 0;

        if (add_state->mod_active)
        {
            float* mod_buf = Sample_get_buffer(add->mod, 0);
            assert(mod_buf != NULL);

            for (int h = 0; h < add_state->mod_tone_limit; ++h)
            {
                if (add->mod_tones[h].pitch_factor <= 0 ||
                        add->mod_tones[h].volume_factor <= 0)
                    continue;

                double pos = add_state->mod_tones[h].phase * BASE_FUNC_SIZE;
                int32_t pos1 = (int)pos & (BASE_FUNC_SIZE - 1);
                int32_t pos2 = (pos1 + 1) & (BASE_FUNC_SIZE - 1);
                float frame = mod_buf[pos1];
                float frame_diff = mod_buf[pos2] - frame;
                double remainder = pos - floor(pos);
                mod_val += (frame + remainder * frame_diff) *
                           add->mod_tones[h].volume_factor * add->mod_volume;

                add_state->mod_tones[h].phase +=
                    actual_pitch *
                    add->mod_tones[h].pitch_factor / freq;

                if (add_state->mod_tones[h].phase >= 1)
                    add_state->mod_tones[h].phase -=
                            floor(add_state->mod_tones[h].phase);
            }

            if (add->force_mod_env_enabled && (add->force_mod_env != NULL))
            {
                double force = min(1, actual_force);
                double factor = Envelope_get_value(add->force_mod_env, force);
                assert(isfinite(factor));
                mod_val *= factor;
            }

            if (add->mod_env_enabled && (add->mod_env != NULL))
            {
                if (add->mod_env_scale_amount != 0 &&
                        (actual_pitch != prev_actual_pitch ||
                         isnan(add_state->mod_env_scale)))
                    add_state->mod_env_scale = pow(
                            actual_pitch / add->mod_env_center,
                            add->mod_env_scale_amount);
                else if (isnan(add_state->mod_env_scale))
                    add_state->mod_env_scale = 1;

                double* next_node = Envelope_get_node(
                        add->mod_env,
                        add_state->mod_env_next_node);

                double scale = NAN;

                if ((next_node == NULL) || (add_state->mod_env_pos >= next_node[0]))
                {
                    ++add_state->mod_env_next_node;
                    scale = Envelope_get_value(add->mod_env, add_state->mod_env_pos);

                    if (!isfinite(scale))
                    {
                        scale = Envelope_get_node(
                                add->mod_env, Envelope_node_count(add->mod_env) - 1)[1];
                        if (scale == 0)
                            add_state->mod_active = false;
                    }
                    else
                    {
                        double next_scale = Envelope_get_value(add->mod_env,
                                                    add_state->mod_env_pos +
                                                    1.0 / freq);
                        add_state->mod_env_value = scale;
                        add_state->mod_env_update = next_scale - scale;
                    }
                }
                else
                {
                    assert(isfinite(add_state->mod_env_update));
                    add_state->mod_env_value += add_state->mod_env_update *
                                                add_state->mod_env_scale;
                    scale = add_state->mod_env_value;
                    if (scale < 0)
                        scale = 0;
                }
                add_state->mod_env_pos += add_state->mod_env_scale / freq;
                mod_val *= scale;
            }

            if (mod_val < 0)
                mod_val += floor(mod_val);
        }

        float* base_buf = Sample_get_buffer(add->base, 0);
        assert(base_buf != NULL);
        for (int h = 0; h < add_state->tone_limit; ++h)
        {
            if (add->tones[h].pitch_factor <= 0 ||
                    add->tones[h].volume_factor <= 0)
                continue;

            // FIXME: + mod_val is specifically phase modulation
            double actual_phase = add_state->tones[h].phase + mod_val;
            double pos = actual_phase * BASE_FUNC_SIZE;
            int32_t pos1 = (int)pos & (BASE_FUNC_SIZE - 1);
            int32_t pos2 = (pos1 + 1) & (BASE_FUNC_SIZE - 1);
            float frame = base_buf[pos1];
            float frame_diff = base_buf[pos2] - frame;
            double remainder = pos - floor(pos);
            double val =
                (frame + remainder * frame_diff) *
                add->tones[h].volume_factor;

            vals[0] += val * (1 - add->tones[h].panning);
            vals[1] += val * (1 + add->tones[h].panning);

            add_state->tones[h].phase += actual_pitch *
                                         add->tones[h].pitch_factor / freq;
            if (add_state->tones[h].phase >= 1)
                add_state->tones[h].phase -= floor(add_state->tones[h].phase);
        }

        audio_l[mixed] = vals[0] * actual_force;
        audio_r[mixed] = vals[1] * actual_force;

        //Generator_common_handle_force(gen, ins_state, vstate, vals, 2, freq);
        //Generator_common_handle_filter(gen, vstate, vals, 2, freq);
        //Generator_common_ramp_attack(gen, vstate, vals, 2, freq);

        //Generator_common_handle_panning(gen, vstate, vals, 2);

        //bufs[0][mixed] += vals[0];
        //bufs[1][mixed] += vals[1];
    }

    const int32_t release_limit = Generator_common_ramp_release(
            gen, ins_state, vstate, wbs, 2, freq, nframes, offset);
    if (release_limit < (int32_t)nframes)
        nframes = release_limit;
    const bool ramp_release_ended = (vstate->ramp_release >= 1);

    Generator_common_handle_filter(gen, vstate, wbs, 2, freq, nframes, offset);
    Generator_common_ramp_attack(gen, vstate, wbs, 2, freq, nframes, offset);
    Generator_common_handle_panning(gen, vstate, wbs, nframes, offset);

    vstate->pos = 1; // XXX: hackish

    for (uint32_t i = offset; i < nframes; ++i)
        bufs[0][i] += audio_l[i];
    for (uint32_t i = offset; i < nframes; ++i)
        bufs[1][i] += audio_r[i];

    if (force_ended || ramp_release_ended)
        vstate->active = false;

    return mixed;
}


static double sine(double phase, double modifier)
{
    (void)modifier;
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


static bool Generator_add_set_base(
        Device_impl* dimpl, Key_indices indices, const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    fill_buf(Sample_get_buffer(add->base, 0), value);

    return true;
}


static bool Generator_add_set_mod_base(
        Device_impl* dimpl, Key_indices indices, const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    fill_buf(Sample_get_buffer(add->mod, 0), value);

    return true;
}


static bool Generator_add_set_mod(
        Device_impl* dimpl, Key_indices indices, int64_t value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    if (value >= MOD_DISABLED && value < MOD_LIMIT)
        add->mod_mode = value;
    else
        add->mod_mode = MOD_DISABLED;

    return true;
}


static bool Generator_add_set_mod_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    if (isfinite(value))
        add->mod_volume = exp2(value / 6.0);
    else
        add->mod_volume = 1.0;

    return true;
}


static bool Generator_add_set_mod_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool enabled)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;
    add->mod_env_enabled = enabled;

    return true;
}


static bool Generator_add_set_mod_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

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


static bool Generator_add_set_mod_env_scale_amount(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    add->mod_env_scale_amount = isfinite(value) ? value : 0.0;

    return true;
}


static bool Generator_add_set_mod_env_scale_center(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

    add->mod_env_center = isfinite(value) ? exp2(value / 1200) * 440 : 440;

    return true;
}


static bool Generator_add_set_force_mod_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;
    add->force_mod_env_enabled = value;

    return true;
}


static bool Generator_add_set_force_mod_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_add* add = (Generator_add*)dimpl;

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


static bool Generator_add_set_tone_pitch(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Generator_add* add = (Generator_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->tones[ti].pitch_factor = value;
    else
        add->tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Generator_add_set_tone_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Generator_add* add = (Generator_add*)dimpl;

    if (isfinite(value))
        add->tones[ti].volume_factor = exp2(value / 6.0);
    else
        add->tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Generator_add_set_tone_panning(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Generator_add* add = (Generator_add*)dimpl;

    if (value >= -1.0 && value <= 1.0)
        add->tones[ti].panning = value;
    else
        add->tones[ti].panning = 0.0;

    return true;
}


static bool Generator_add_set_mod_tone_pitch(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Generator_add* add = (Generator_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->mod_tones[ti].pitch_factor = value;
    else
        add->mod_tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Generator_add_set_mod_tone_volume(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    int32_t ti = indices[0];
    if (ti < 0 || ti >= HARMONICS_MAX)
        return true;

    Generator_add* add = (Generator_add*)dimpl;

    if (isfinite(value))
        add->mod_tones[ti].volume_factor = exp2(value / 6.0);
    else
        add->mod_tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static void del_Generator_add(Device_impl* gen_impl)
{
    if (gen_impl == NULL)
        return;

    //assert(string_eq(gen->type, "add"));
    Generator_add* add = (Generator_add*)gen_impl;
    del_Sample(add->base);
    del_Sample(add->mod);
    memory_free(add);

    return;
}


