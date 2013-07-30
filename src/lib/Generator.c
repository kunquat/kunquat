

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <memory.h>
#include <pitch_t.h>
#include <xassert.h>


static Device_state* Generator_create_state_plain(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Gen_state* gen_state = memory_alloc_item(Gen_state);
    if (gen_state == NULL)
        return NULL;

    Gen_state_init(gen_state, device, audio_rate, audio_buffer_size);

    return &gen_state->parent;
}


Generator* new_Generator(
        char* str,
        const Instrument_params* ins_params,
        uint32_t buffer_size,
        uint32_t mix_rate,
        Read_state* state)
{
    assert(str != NULL);
    assert(ins_params != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    assert(state != NULL);

    if (state->error)
        return NULL;

    char type[GEN_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, GEN_TYPE_LENGTH_MAX, state);
    if (state->error)
        return NULL;

    Generator_cons* cons = Gen_type_find_cons(type);
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported Generator type: %s", type);
        return NULL;
    }

    Generator* gen = cons(buffer_size, mix_rate);
    if (gen == NULL)
        return NULL;

    //fprintf(stderr, "New Generator %p\n", (void*)gen);
    strcpy(gen->type, type);
    gen->ins_params = ins_params;
    gen->conf = NULL;

    Device_set_state_creator(
            &gen->parent,
            Generator_create_state_plain);

    return gen;
}


bool Generator_init(
        Generator* gen,
        void (*destroy)(Generator*),
        uint32_t (*mix)(
            Generator*,
            Gen_state*,
            Ins_state*,
            Voice_state*,
            uint32_t,
            uint32_t,
            uint32_t,
            double),
        void (*init_vstate)(Generator*, const Gen_state*, Voice_state*),
        uint32_t buffer_size,
        uint32_t mix_rate)
{
    assert(gen != NULL);
    assert(destroy != NULL);
    assert(mix != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    gen->destroy = destroy;
    gen->mix = mix;
    gen->init_vstate = init_vstate;

    if (!Device_init(&gen->parent, buffer_size, mix_rate))
        return false;

    Device_set_reset(&gen->parent, Generator_reset);
    Device_register_port(&gen->parent, DEVICE_PORT_TYPE_SEND, 0);
    return true;
}


void Generator_reset(Device* device)
{
    assert(device != NULL);

    Generator* gen = (Generator*)device;
    Device_params_reset(gen->conf->params);

    return;
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


void Generator_mix(
        Generator* gen,
        Device_states* dstates,
        Voice_state* vstate,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    assert(dstates != NULL);
    assert(vstate != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    if (offset < nframes)
    {
        Gen_state* gen_state = (Gen_state*)Device_states_get_state(
                dstates,
                Device_get_id(&gen->parent));
        Ins_state* ins_state = (Ins_state*)Device_states_get_state(
                dstates,
                gen->ins_params->device_id);

        gen->mix(gen, gen_state, ins_state, vstate, nframes, offset, freq, tempo);
    }

    return;
}


void del_Generator(Generator* gen)
{
    if (gen == NULL)
        return;

    assert(gen->destroy != NULL);
    Device_deinit(&gen->parent);
    gen->destroy(gen);

    return;
}


