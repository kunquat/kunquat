

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <Generator.h>
#include <Generator_sine.h>
#include <Generator_sawtooth.h>
#include <Generator_triangle.h>
#include <Generator_pulse.h>
#include <Generator_square303.h>
#include <Generator_pcm.h>
#include <Generator_noise.h>
#include <File_base.h>
#include <Filter.h>
#include <Event_ins.h>
#include <pitch_t.h>
#include <Random.h>
#include <xassert.h>
#include <xmemory.h>


Generator* new_Generator(Gen_type type,
                         Instrument_params* ins_params,
                         Device_params* gen_params,
                         uint32_t buffer_size)
{
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    assert(ins_params != NULL);
    assert(gen_params != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    static Generator* (*cons[])(Instrument_params*, Device_params*) =
    {
        [GEN_TYPE_SINE] = new_Generator_sine,
        [GEN_TYPE_SAWTOOTH] = new_Generator_sawtooth,
        [GEN_TYPE_TRIANGLE] = new_Generator_triangle,
        [GEN_TYPE_PULSE] = new_Generator_pulse,
        [GEN_TYPE_SQUARE303] = new_Generator_square303,
        [GEN_TYPE_NOISE] = new_Generator_noise,
        [GEN_TYPE_PCM] = new_Generator_pcm,
    };
    assert(cons[type] != NULL);
    Generator* gen = cons[type](ins_params, gen_params);
    if (gen == NULL)
    {
        return NULL;
    }
    if (!Device_init(&gen->parent, buffer_size))
    {
        del_Generator(gen);
        return NULL;
    }
    Device_register_port(&gen->parent, DEVICE_PORT_TYPE_SEND, 0);
//    if (type == GEN_TYPE_PCM) fprintf(stderr, "returning new pcm %p\n", (void*)gen);
    return gen;
}


bool Generator_init(Generator* gen)
{
    assert(gen != NULL);
    gen->enabled = GENERATOR_DEFAULT_ENABLED;
    gen->volume_dB = GENERATOR_DEFAULT_VOLUME;
    gen->volume = exp2(gen->volume_dB / 6);
    gen->pitch_lock_enabled = GENERATOR_DEFAULT_PITCH_LOCK_ENABLED;
    gen->pitch_lock_cents = GENERATOR_DEFAULT_PITCH_LOCK_CENTS;
    gen->pitch_lock_freq = exp2(gen->pitch_lock_cents / 1200.0) * 440;
    gen->type_params = NULL;
    return true;
}


void Generator_uninit(Generator* gen)
{
    assert(gen != NULL);
    (void)gen;
    return;
}


Device_params* Generator_get_params(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type_params != NULL);
    return gen->type_params;
}


void Generator_copy_general(Generator* dest, Generator* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    dest->enabled = src->enabled;
    dest->volume_dB = src->volume_dB;
    dest->volume = src->volume;
    dest->pitch_lock_enabled = src->pitch_lock_enabled;
    dest->pitch_lock_cents = src->pitch_lock_cents;
    dest->pitch_lock_freq = src->pitch_lock_freq;
    dest->random = src->random;
    dest->type_params = src->type_params;
    return;
}


bool Generator_parse_general(Generator* gen, char* str, Read_state* state)
{
    assert(gen != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    bool enabled = false;
    double volume = 0;
    bool pitch_lock_enabled = GENERATOR_DEFAULT_PITCH_LOCK_ENABLED;
    double pitch_lock_cents = GENERATOR_DEFAULT_PITCH_LOCK_CENTS;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            char key[128] = { '\0' };
            bool expect_key = true;
            while (expect_key)
            {
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (strcmp(key, "enabled") == 0)
                {
                    str = read_bool(str, &enabled, state);
                }
                else if (strcmp(key, "volume") == 0)
                {
                    str = read_double(str, &volume, state);
                }
                else if (strcmp(key, "pitch_lock") == 0)
                {
                    str = read_bool(str, &pitch_lock_enabled, state);
                }
                else if (strcmp(key, "pitch_lock_cents") == 0)
                {
                    str = read_double(str, &pitch_lock_cents, state);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unsupported key in Generator info: %s", key);
                    return false;
                }
                if (state->error)
                {
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    gen->enabled = enabled;
    gen->volume_dB = volume;
    gen->volume = exp2(gen->volume_dB / 6);
    gen->pitch_lock_enabled = pitch_lock_enabled;
    gen->pitch_lock_cents = pitch_lock_cents;
    gen->pitch_lock_freq = exp2(gen->pitch_lock_cents / 1200.0) * 440;
    return true;
}


bool Generator_parse_param(Generator* gen,
                           const char* subkey,
                           void* data,
                           long length,
                           Read_state* state)
{
    assert(gen != NULL);
    assert(subkey != NULL);
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    return Device_params_parse_value(gen->type_params, subkey,
                                     data, length, state);
}


Gen_type Generator_type_parse(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return GEN_TYPE_LAST;
    }
    if (str == NULL)
    {
        return GEN_TYPE_NONE;
    }
    static const char* map[GEN_TYPE_LAST] =
    {
        [GEN_TYPE_SINE] = "sine",
        [GEN_TYPE_TRIANGLE] = "triangle",
        [GEN_TYPE_PULSE] = "pulse",
        [GEN_TYPE_SQUARE303] = "square303",
        [GEN_TYPE_SAWTOOTH] = "sawtooth",
        [GEN_TYPE_NOISE] = "noise",
        [GEN_TYPE_PCM] = "pcm",
    };
    char desc[128] = { '\0' };
    str = read_string(str, desc, 128, state);
    if (state->error)
    {
        return GEN_TYPE_LAST;
    }
    for (Gen_type i = GEN_TYPE_NONE; i < GEN_TYPE_LAST; ++i)
    {
        if (map[i] != NULL && strcmp(map[i], desc) == 0)
        {
            return i;
        }
    }
    Read_state_set_error(state, "Unsupported Generator type: %s", desc);
    return GEN_TYPE_LAST;
}


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_process_note(Generator* gen,
                            Voice_state* state,
                            double cents)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(isfinite(cents));
    if (gen->ins_params->pitch_lock_enabled)
    {
        state->pitch = gen->ins_params->pitch_lock_freq;
        return;
    }
    if (gen->ins_params->scale == NULL ||
            *gen->ins_params->scale == NULL ||
            **gen->ins_params->scale == NULL)
    {
        state->pitch = cents;
        return;
    }
    pitch_t pitch = Scale_get_pitch_from_cents(**gen->ins_params->scale, cents);
    if (pitch > 0)
    {
        state->pitch = pitch;
    }
    return;
}


void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq,
                   double tempo)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    if (offset < nframes)
    {
        gen->mix(gen, state, nframes, offset, freq, tempo);
    }
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    Device_uninit(&gen->parent);
    gen->destroy(gen);
    return;
}


