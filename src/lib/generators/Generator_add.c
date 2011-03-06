

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


typedef double (*Base_func)(double phase, double modifier);


#define BASE_FUNC_SIZE 4096


typedef enum
{
    BASE_FUNC_ID_SINE = 0,
    BASE_FUNC_ID_SAWTOOTH,
    BASE_FUNC_ID_PULSE,
    BASE_FUNC_ID_TRIANGLE,
    BASE_FUNC_ID_LIMIT
} Base_func_id;


typedef struct Add_tone
{
    double pitch_factor;
    double volume_factor;
} Add_tone;


typedef struct Generator_add
{
    Generator parent;
    Sample* base;
    Base_func base_func;
    double detune;
    Add_tone tones[HARMONICS_MAX];
} Generator_add;


static void Generator_add_init_state(Generator* gen, Voice_state* state);

static double sine(double phase, double modifier);
static double sawtooth(double phase, double modifier);
static double pulse(double phase, double modifier);
static double triangle(double phase, double modifier);

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
    add->base = NULL;
    add->base_func = sine;
    add->detune = 1;
    float* buf = xnalloc(float, BASE_FUNC_SIZE);
    if (buf == NULL)
    {
        del_Generator(&add->parent);
        return NULL;
    }
    add->base = new_Sample_from_buffers(&buf, 1, BASE_FUNC_SIZE);
    if (add->base == NULL)
    {
        xfree(buf);
        del_Generator(&add->parent);
        return NULL;
    }
    Sample_set_loop_start(add->base, 0);
    Sample_set_loop_end(add->base, BASE_FUNC_SIZE);
    Sample_set_loop(add->base, SAMPLE_LOOP_UNI);
    for (int i = 0; i < BASE_FUNC_SIZE; ++i)
    {
        buf[i] = add->base_func((double)i / BASE_FUNC_SIZE, 0);
    }
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
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
    (void)gen;
    assert(state != NULL);
    Voice_state_add* add_state = (Voice_state_add*)state;
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        add_state->tones[h].pos = 0;
        add_state->tones[h].pos_rem = 0;
        add_state->tones[h].rel_pos = 0;
        add_state->tones[h].rel_pos_rem = 0;
        for (int i = 0; i < FILTER_ORDER; ++i)
        {
            add_state->tones[h].lowpass_state[0].coeffs[i] = 0;
            add_state->tones[h].lowpass_state[1].coeffs[i] = 0;
            for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
            {
                add_state->tones[h].lowpass_state[0].history1[k][i] = 0;
                add_state->tones[h].lowpass_state[0].history2[k][i] = 0;
                add_state->tones[h].lowpass_state[1].history1[k][i] = 0;
                add_state->tones[h].lowpass_state[1].history2[k][i] = 0;
            }
        }
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
    Voice_state* hstate = &(Voice_state){ .active = false };
    uint32_t mixed = 0;
    bool active = true;
    bool hfound = false;
    for (int h = 0; h < HARMONICS_MAX; ++h)
    {
        if (add->tones[h].pitch_factor <= 0 ||
                add->tones[h].volume_factor <= 0)
        {
            continue;
        }
        hfound = true;
        memcpy(hstate, state, sizeof(Voice_state));
        hstate->pos = add_state->tones[h].pos;
        hstate->pos_rem = add_state->tones[h].pos_rem;
        hstate->rel_pos = add_state->tones[h].rel_pos;
        hstate->rel_pos_rem = add_state->tones[h].rel_pos_rem;
        memcpy(hstate->lowpass_state, add_state->tones[h].lowpass_state,
                2 * sizeof(Filter_state));
        uint32_t hmixed = Sample_mix(add->base, gen, hstate, nframes,
                                     offset, freq, tempo, bufs,
                                     1.0 / add->tones[h].pitch_factor,
                                     BASE_FUNC_SIZE,
                                     add->tones[h].volume_factor);
        active &= hstate->active;
        add_state->tones[h].pos = hstate->pos;
        add_state->tones[h].pos_rem = hstate->pos_rem;
        add_state->tones[h].rel_pos = hstate->rel_pos;
        add_state->tones[h].rel_pos_rem = hstate->rel_pos_rem;
        memcpy(add_state->tones[h].lowpass_state, hstate->lowpass_state,
                2 * sizeof(Filter_state));
        if (hmixed > mixed)
        {
            mixed = hmixed;
        }
    }
    if (hfound)
    {
        memcpy(state, hstate, sizeof(Voice_state));
    }
    add_state->parent.active = active;
    return mixed;
#if 0
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = add->base_func(add_state->phase, 0);

        Generator_common_handle_force(gen, state, vals, 1, freq);
        Generator_common_handle_filter(gen, state, vals, 1, freq);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        add_state->phase += state->actual_pitch / freq;
        if (add_state->phase >= 1)
        {
            add_state->phase -= floor(add_state->phase);
        }
        state->pos = 1; // XXX: hackish

        vals[1] = vals[0];
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
    }
    return mixed;
#endif
}


static double sine(double phase, double modifier)
{
    (void)modifier;
    return sin(phase * PI * 2);
}


static double sawtooth(double phase, double modifier)
{
    (void)modifier;
    return (phase * 2) - 1;
}


static double pulse(double phase, double modifier)
{
    if ((phase - 0.5) < modifier)
    {
        return 1.0;
    }
    return -1.0;
}


static double triangle(double phase, double modifier)
{
    (void)modifier;
    if (phase < 0.5)
    {
        return (phase * 4) - 1;
    }
    return (phase * (-4)) + 3;
}


static bool Generator_add_sync(Device* device)
{
    assert(device != NULL);
    Generator_add_update_key(device, "p_bfunc.jsoni");
    Generator_add_update_key(device, "p_base.jsonln");
    char pitch_key[] = "tone_XX/p_pitch.jsonf";
    int pitch_key_bytes = strlen(pitch_key) + 1;
    char volume_key[] = "tone_XX/p_volume.jsonf";
    int volume_key_bytes = strlen(volume_key) + 1;
    for (int i = 0; i < HARMONICS_MAX; ++i)
    {
        snprintf(pitch_key, pitch_key_bytes, "tone_%02x/p_pitch.jsonf", i);
        Generator_add_update_key(device, pitch_key);
        snprintf(volume_key, volume_key_bytes, "tone_%02x/p_volume.jsonf", i);
        Generator_add_update_key(device, volume_key);
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
    if (string_eq(key, "p_bfunc.jsoni"))
    {
        static const Base_func base_funcs[BASE_FUNC_ID_LIMIT] =
        {
            [BASE_FUNC_ID_SINE] = sine,
            [BASE_FUNC_ID_SAWTOOTH] = sawtooth,
            [BASE_FUNC_ID_PULSE] = pulse,
            [BASE_FUNC_ID_TRIANGLE] = triangle,
        };
        int64_t* bfunc = Device_params_get_int(params, key);
        if (bfunc != NULL && *bfunc >= 0 && *bfunc < BASE_FUNC_ID_LIMIT)
        {
            assert(base_funcs[*bfunc] != NULL);
            add->base_func = base_funcs[*bfunc];
        }
        else
        {
            add->base_func = sine;
        }
    }
    else if (string_eq(key, "p_base.jsonln"))
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
                buf[i] = add->base_func((double)i / BASE_FUNC_SIZE, 0);
            }
        }
    }
    else if ((ti = string_extract_index(key, "tone_", 2,
                                        "/p_pitch.jsonf")) >= 0
             && ti < HARMONICS_MAX)
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
                                        "/p_volume.jsonf")) >= 0
            && ti < HARMONICS_MAX)
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
    xfree(add);
    return;
}


