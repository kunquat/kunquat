

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
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <Generator.h>
#include <Generator_sine.h>
#include <Generator_sawtooth.h>
#include <Generator_triangle.h>
#include <Generator_square.h>
#include <Generator_square303.h>
#include <Generator_pcm.h>
#include <Generator_noise.h>
#include <File_base.h>
#include <Filter.h>
#include <Event_ins.h>
#include <Random.h>

#include <xmemory.h>


Generator* new_Generator(Gen_type type, Instrument_params* ins_params)
{
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    assert(ins_params != NULL);
    Generator* (*cons[])(Instrument_params*) =
    {
        [GEN_TYPE_SINE] = new_Generator_sine,
        [GEN_TYPE_SAWTOOTH] = new_Generator_sawtooth,
        [GEN_TYPE_TRIANGLE] = new_Generator_triangle,
        [GEN_TYPE_SQUARE] = new_Generator_square,
        [GEN_TYPE_SQUARE303] = new_Generator_square303,
        [GEN_TYPE_NOISE] = new_Generator_noise,
        [GEN_TYPE_PCM] = new_Generator_pcm,
    };
    assert(cons[type] != NULL);
    Generator* gen = cons[type](ins_params);
//    if (type == GEN_TYPE_PCM) fprintf(stderr, "returning new pcm %p\n", (void*)gen);
    return gen;
}


bool Generator_init(Generator* gen)
{
    assert(gen != NULL);
    gen->enabled = GENERATOR_DEFAULT_ENABLED;
    gen->volume_dB = GENERATOR_DEFAULT_VOLUME;
    gen->volume = exp2(gen->volume_dB / 6);
    gen->parse = NULL;
    return true;
}


void Generator_uninit(Generator* gen)
{
    assert(gen != NULL);
    (void)gen;
    return;
}


void Generator_copy_general(Generator* dest, Generator* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    dest->enabled = src->enabled;
    dest->volume_dB = src->volume_dB;
    dest->volume = src->volume;
    dest->random = src->random;
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
    return true;
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
        [GEN_TYPE_SQUARE] = "square",
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


bool Generator_type_has_subkey(Gen_type type, const char* subkey)
{
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    if (subkey == NULL)
    {
        return false;
    }
    static bool (*map[GEN_TYPE_LAST])(const char*) =
    {
        [GEN_TYPE_PCM] = Generator_pcm_has_subkey,
        [GEN_TYPE_SQUARE] = Generator_square_has_subkey,
        [GEN_TYPE_NOISE] = Generator_noise_has_subkey,
    };
    if (map[type] == NULL)
    {
        return false;
    }
    return map[type](subkey);
}


bool Generator_parse(Generator* gen,
                     const char* subkey,
                     void* data,
                     long length,
                     Read_state* state)
{
    assert(gen != NULL);
    assert(subkey != NULL);
    assert(Generator_type_has_subkey(Generator_get_type(gen), subkey));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    assert(gen->parse != NULL);
    return gen->parse(gen, subkey, data, length, state);
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
    if (gen->ins_params->scale == NULL ||
            *gen->ins_params->scale == NULL ||
            **gen->ins_params->scale == NULL)
    {
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
    assert(freq > 0);
    assert(tempo > 0);
    gen->mix(gen, state, nframes, offset, freq, tempo,
             gen->ins_params->buf_count, gen->ins_params->bufs);
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


