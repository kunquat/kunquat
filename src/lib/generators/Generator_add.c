

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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

#include <Generator.h>
#include <Generator_add.h>
#include <Generator_common.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <Num_list.h>
#include <Sample.h>
#include <Sample_mix.h>
#include <string_common.h>
#include <Voice_state.h>
#include <Voice_state_add.h>
#include <xassert.h>
#include <xmemory.h>


#define BASE_FUNC_SIZE 4096


typedef struct Add_tone
{
    double pitch_factor;
    double volume_factor;
} Add_tone;


typedef enum
{
    MOD_DISABLED = 0,
    MOD_PHASE,
    MOD_LIMIT
} Mod_mode;


typedef struct Generator_add
{
    Generator parent;
    Sample* base;
    Sample* mod;
    Mod_mode mod_mode;
    double mod_volume;
    double detune;
    Add_tone tones[HARMONICS_MAX];
    Add_tone mod_tones[HARMONICS_MAX];
} Generator_add;


static void Generator_add_init_state(Generator* gen, Voice_state* state);

static double sine(double phase, double modifier);

static bool Generator_add_sync(Device* device);
static bool Generator_add_update_key(Device* device, const char* key);

static uint32_t Generator_add_mix(Generator* gen,
                                  Voice_state* state,
                                  uint32_t nframes,
                                  uint32_t offset,
                                  uint32_t freq,
                                  double tempo);

static void del_Generator_add(Generator* gen);


Generator* new_Generator_add(uint32_t buffer_size,
                             uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    Generator_add* add = xalloc(Generator_add);
    if (add == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&add->parent,
                        del_Generator_add,
                        Generator_add_mix,
                        Generator_add_init_state,
                        buffer_size,
                        mix_rate))
    {
        xfree(add);
        return NULL;
    }
    Device_set_sync(&add->parent.parent, Generator_add_sync);
    Device_set_update_key(&add->parent.parent, Generator_add_update_key);
    add->base = NULL;
    add->mod = NULL;
    add->mod_mode = MOD_DISABLED;
    add->mod_volume = 1;
    add->detune = 1;
    float* buf = xnalloc(float, BASE_FUNC_SIZE);
    float* mod_buf = xnalloc(float, BASE_FUNC_SIZE);
    if (buf == NULL || mod_buf == NULL)
    {
        xfree(buf);
        xfree(mod_buf);
        del_Generator(&add->parent);
        return NULL;
    }
    add->base = new_Sample_from_buffers(&buf, 1, BASE_FUNC_SIZE);
    if (add->base == NULL)
    {
        xfree(buf);
        xfree(mod_buf);
        del_Generator(&add->parent);
        return NULL;
    }
    add->mod = new_Sample_from_buffers(&mod_buf, 1, BASE_FUNC_SIZE);
    if (add->mod == NULL)
    {
        xfree(mod_buf);
        del_Generator(&add->parent);
        return NULL;
    }
    Sample_set_loop_start(add->base, 0);
    Sample_set_loop_end(add->base, BASE_FUNC_SIZE);
    Sample_set_loop(add->base, SAMPLE_LOOP_UNI);
    for (int i = 0; i < BASE_FUNC_SIZE; ++i)
    {
        buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
        mod_buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
    }
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
        add->mod_tones[h].pitch_factor = 0;
        add->mod_tones[h].volume_factor = 0;
    }
    return &add->parent;
}


char* Generator_add_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "add"));
    assert(property_type != NULL);
    (void)gen;
    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
        {
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_add));
        }
        return size_str;
    }
    return NULL;
}


static void Generator_add_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "add"));
    assert(state != NULL);
    Generator_add* add = (Generator_add*)gen;
    Voice_state_add* add_state = (Voice_state_add*)state;
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        if (add->tones[h].pitch_factor <= 0 ||
                add->tones[h].volume_factor <= 0)
        {
            continue;
        }
        add_state->tone_limit = h + 1;
        add_state->tones[h].phase = 0;
    }
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        if (add->mod_tones[h].pitch_factor <= 0 ||
                add->mod_tones[h].volume_factor <= 0)
        {
            continue;
        }
        add_state->mod_tone_limit = h + 1;
        add_state->mod_tones[h].phase = 0;
    }
    return;
}


static uint32_t Generator_add_mix(Generator* gen,
                                  Voice_state* state,
                                  uint32_t nframes,
                                  uint32_t offset,
                                  uint32_t freq,
                                  double tempo)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "add"));
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    Generator_add* add = (Generator_add*)gen;
    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen, state, offset, bufs);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
    Voice_state_add* add_state = (Voice_state_add*)state;
    uint32_t mixed = offset;
    assert(is_p2(BASE_FUNC_SIZE));
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);
        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = 0;
        double mod_val = 0;
        if (add->mod_mode)
        {
            float* mod_buf = Sample_get_buffer(add->mod, 0);
            assert(mod_buf != NULL);
            for (int h = 0; h < add_state->mod_tone_limit; ++h)
            {
                if (add->mod_tones[h].pitch_factor <= 0 ||
                        add->mod_tones[h].volume_factor <= 0)
                {
                    continue;
                }
                double pos = add_state->mod_tones[h].phase * BASE_FUNC_SIZE;
                int32_t pos1 = (int)pos & (BASE_FUNC_SIZE - 1);
                int32_t pos2 = (pos1 + 1) & (BASE_FUNC_SIZE - 1);
                float frame = mod_buf[pos1];
                float frame_diff = mod_buf[pos2] - frame;
                double remainder = pos - floor(pos);
                mod_val += (frame + remainder * frame_diff) *
                           add->mod_tones[h].volume_factor * add->mod_volume;
                add_state->mod_tones[h].phase += state->actual_pitch *
                                        add->mod_tones[h].pitch_factor / freq;
                if (add_state->mod_tones[h].phase >= 1)
                {
                    add_state->mod_tones[h].phase -=
                            floor(add_state->mod_tones[h].phase);
                }
            }
            if (mod_val < 0)
            {
                mod_val += floor(mod_val);
            }
        }
        float* base_buf = Sample_get_buffer(add->base, 0);
        assert(base_buf);
        for (int h = 0; h < add_state->tone_limit; ++h)
        {
            if (add->tones[h].pitch_factor <= 0 ||
                    add->tones[h].volume_factor <= 0)
            {
                continue;
            }
            // FIXME: + mod_val is specifically phase modulation
            double actual_phase = add_state->tones[h].phase + mod_val;
            double pos = actual_phase * BASE_FUNC_SIZE;
            int32_t pos1 = (int)pos & (BASE_FUNC_SIZE - 1);
            int32_t pos2 = (pos1 + 1) & (BASE_FUNC_SIZE - 1);
            float frame = base_buf[pos1];
            float frame_diff = base_buf[pos2] - frame;
            double remainder = pos - floor(pos);
            vals[0] += (frame + remainder * frame_diff) *
                       add->tones[h].volume_factor;
            add_state->tones[h].phase += state->actual_pitch *
                                         add->tones[h].pitch_factor / freq;
            if (add_state->tones[h].phase >= 1)
            {
                add_state->tones[h].phase -= floor(add_state->tones[h].phase);
            }
        }
        Generator_common_handle_force(gen, state, vals, 1, freq);
        Generator_common_handle_filter(gen, state, vals, 1, freq);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        state->pos = 1; // XXX: hackish
        vals[1] = vals[0];
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
    }
    return mixed;
}


static double sine(double phase, double modifier)
{
    (void)modifier;
    return sin(phase * PI * 2);
}


static bool Generator_add_sync(Device* device)
{
    assert(device != NULL);
    Generator_add_update_key(device, "p_bfunc.jsoni");
    Generator_add_update_key(device, "p_base.jsonln");
    Generator_add_update_key(device, "p_mod.jsoni");
    Generator_add_update_key(device, "p_mod_volume.jsonf");
    char pitch_key[] = "tone_XX/p_pitch.jsonf";
    int pitch_key_bytes = strlen(pitch_key) + 1;
    char volume_key[] = "tone_XX/p_volume.jsonf";
    int volume_key_bytes = strlen(volume_key) + 1;
    char mod_pitch_key[] = "mod_XX/p_pitch.jsonf";
    int mod_pitch_key_bytes = strlen(mod_pitch_key) + 1;
    char mod_volume_key[] = "mod_XX/p_volume.jsonf";
    int mod_volume_key_bytes = strlen(mod_volume_key) + 1;
    for (int i = 0; i < HARMONICS_MAX; ++i)
    {
        snprintf(pitch_key, pitch_key_bytes, "tone_%02x/p_pitch.jsonf", i);
        Generator_add_update_key(device, pitch_key);
        snprintf(volume_key, volume_key_bytes, "tone_%02x/p_volume.jsonf", i);
        Generator_add_update_key(device, volume_key);
        snprintf(mod_pitch_key, mod_pitch_key_bytes,
                 "mod_%02x/p_pitch.jsonf", i);
        Generator_add_update_key(device, mod_pitch_key);
        snprintf(mod_volume_key, mod_volume_key_bytes,
                 "mod_%02x/p_volume.jsonf", i);
        Generator_add_update_key(device, mod_volume_key);
    }
    return true;
}


static bool Generator_add_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);
    Generator_add* add = (Generator_add*)device;
    Device_params* params = add->parent.conf->params;
    int ti = -1;
    if (string_eq(key, "p_base.jsonln"))
    {
        Num_list* nl = Device_params_get_num_list(params, key);
        float* buf = Sample_get_buffer(add->base, 0);
        assert(buf != NULL);
        if (nl != NULL)
        {
            int32_t available = MIN(Num_list_length(nl), BASE_FUNC_SIZE);
            for (int i = 0; i < available; ++i)
            {
                buf[i] = MAX(-1.0, MIN(1.0, Num_list_get_num(nl, i)));
            }
            for (int i = available; i < BASE_FUNC_SIZE; ++i)
            {
                buf[i] = 0;
            }
        }
        else
        {
            for (int i = 0; i < BASE_FUNC_SIZE; ++i)
            {
                buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
            }
        }
    }
    else if (string_eq(key, "p_mod_base.jsonln"))
    {
        Num_list* nl = Device_params_get_num_list(params, key);
        float* buf = Sample_get_buffer(add->mod, 0);
        assert(buf != NULL);
        if (nl != NULL)
        {
            int32_t available = MIN(Num_list_length(nl), BASE_FUNC_SIZE);
            for (int i = 0; i < available; ++i)
            {
                buf[i] = MAX(-1.0, MIN(1.0, Num_list_get_num(nl, i)));
            }
            for (int i = available; i < BASE_FUNC_SIZE; ++i)
            {
                buf[i] = 0;
            }
        }
        else
        {
            for (int i = 0; i < BASE_FUNC_SIZE; ++i)
            {
                buf[i] = sine((double)i / BASE_FUNC_SIZE, 0);
            }
        }
    }
    else if (string_eq(key, "p_mod.jsoni"))
    {
        int64_t* mode = Device_params_get_int(params, key);
        if (mode != NULL && *mode >= MOD_DISABLED && *mode < MOD_LIMIT)
        {
            add->mod_mode = *mode;
        }
        else
        {
            add->mod_mode = MOD_DISABLED;
        }
    }
    else if (string_eq(key, "p_mod_volume.jsonf"))
    {
        double* volume_dB = Device_params_get_float(params, key);
        if (volume_dB != NULL && isfinite(*volume_dB))
        {
            add->mod_volume = exp2(*volume_dB / 6);
        }
        else
        {
            add->mod_volume = 1;
        }
    }
    else if ((ti = string_extract_index(key, "tone_", 2,
                                        "/p_pitch.jsonf")) >= 0 &&
             ti < HARMONICS_MAX)
    {
        double* pitch = Device_params_get_float(params, key);
        if (pitch != NULL && *pitch > 0 && isfinite(*pitch))
        {
            add->tones[ti].pitch_factor = *pitch;
        }
        else
        {
            add->tones[ti].pitch_factor = ti == 0 ? 1 : 0;
        }
    }
    else if ((ti = string_extract_index(key, "tone_", 2,
                                        "/p_volume.jsonf")) >= 0 &&
             ti < HARMONICS_MAX)
    {
        double* volume_dB = Device_params_get_float(params, key);
        if (volume_dB != NULL && isfinite(*volume_dB))
        {
            add->tones[ti].volume_factor = exp2(*volume_dB / 6);
        }
        else
        {
            add->tones[ti].volume_factor = ti == 0 ? 1 : 0;
        }
    }
    else if ((ti = string_extract_index(key, "mod_", 2,
                                        "/p_pitch.jsonf")) >= 0 &&
             ti < HARMONICS_MAX)
    {
        double* pitch = Device_params_get_float(params, key);
        if (pitch != NULL && *pitch > 0 && isfinite(*pitch))
        {
            add->mod_tones[ti].pitch_factor = *pitch;
        }
        else
        {
            add->mod_tones[ti].pitch_factor = ti == 0 ? 1 : 0;
        }
    }
    else if ((ti = string_extract_index(key, "mod_", 2,
                                        "/p_volume.jsonf")) >= 0 &&
             ti < HARMONICS_MAX)
    {
        double* volume_dB = Device_params_get_float(params, key);
        if (volume_dB != NULL && isfinite(*volume_dB))
        {
            add->mod_tones[ti].volume_factor = exp2(*volume_dB / 6);
        }
        else
        {
            add->mod_tones[ti].volume_factor = ti == 0 ? 1 : 0;
        }
    }
    return true;
}


static void del_Generator_add(Generator* gen)
{
    if (gen == NULL)
    {
        return;
    }
    assert(string_eq(gen->type, "add"));
    Generator_add* add = (Generator_add*)gen;
    del_Sample(add->base);
    del_Sample(add->mod);
    xfree(add);
    return;
}


