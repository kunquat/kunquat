

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

#include <Gen_type.h>
#include <Generator.h>
#include <File_base.h>
#include <Filter.h>
#include <Event_ins.h>
#include <pitch_t.h>
#include <Random.h>
#include <xassert.h>
#include <xmemory.h>


Generator* new_Generator(char* str,
                         Instrument_params* ins_params,
                         uint32_t buffer_size,
                         uint32_t mix_rate,
                         Random* random,
                         Read_state* state)
{
    assert(str != NULL);
    assert(ins_params != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    assert(random != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    char type[GEN_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, GEN_TYPE_LENGTH_MAX, state);
    if (state->error)
    {
        return NULL;
    }
    Generator* (*cons)(uint32_t, uint32_t) = Gen_type_find_cons(type);
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported Generator type: %s", type);
        return NULL;
    }
    Generator* gen = cons(buffer_size, mix_rate);
    if (gen == NULL)
    {
        return NULL;
    }
    strcpy(gen->type, type);
    gen->ins_params = ins_params;
    gen->conf = NULL;
    gen->random = random;
    return gen;
}


bool Generator_init(Generator* gen,
                    void (*destroy)(Generator*),
                    uint32_t (*mix)(Generator*, Voice_state*, uint32_t, uint32_t, uint32_t, double),
                    void (*init_state)(Generator*, Voice_state*),
                    uint32_t buffer_size,
                    uint32_t mix_rate)
{
    assert(gen != NULL);
    assert(destroy != NULL);
    assert(mix != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    gen->destroy = destroy;
    gen->mix = mix;
    gen->init_state = init_state;
    if (!Device_init(&gen->parent, buffer_size, mix_rate))
    {
        return false;
    }
    Device_register_port(&gen->parent, DEVICE_PORT_TYPE_SEND, 0);
    return true;
}


void Generator_set_conf(Generator* gen, Gen_conf* conf)
{
    assert(gen != NULL);
    assert(conf != NULL);
    gen->conf = conf;
    return;
}


Device_params* Generator_get_params(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->conf != NULL);
    assert(gen->conf->params != NULL);
    return gen->conf->params;
}


char* Generator_get_type(Generator* gen)
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
    if (gen == NULL)
    {
        return;
    }
    assert(gen->destroy != NULL);
    Device_uninit(&gen->parent);
    gen->destroy(gen);
    return;
}


