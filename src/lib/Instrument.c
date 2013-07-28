

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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Device.h>
#include <Effect.h>
#include <Effect_table.h>
#include <File_base.h>
#include <Gen_table.h>
#include <Generator.h>
#include <Instrument.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


struct Instrument
{
    Device parent;

    Connections* connections;

//    double default_force;       ///< Default force.

    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Generators need.

    Gen_table* gens;
    Effect_table* effects;
};


static void Instrument_reset(Device* device);

static bool Instrument_set_mix_rate(Device* device, uint32_t mix_rate);

static bool Instrument_set_buffer_size(Device* device, uint32_t size);

static bool Instrument_sync(Device* device);


Instrument* new_Instrument(uint32_t buf_len,
                           uint32_t mix_rate)
{
    assert(buf_len > 0);
    assert(mix_rate > 0);

    Instrument* ins = memory_alloc_item(Instrument);
    if (ins == NULL)
        return NULL;

    //fprintf(stderr, "New Instrument %p\n", (void*)ins);
    ins->connections = NULL;
    ins->gens = NULL;
    ins->effects = NULL;

    if (Instrument_params_init(&ins->params) == NULL)
    {
        memory_free(ins);
        return NULL;
    }
    if (!Device_init(&ins->parent, buf_len, mix_rate))
    {
        Instrument_params_uninit(&ins->params);
        memory_free(ins);
        return NULL;
    }
    Device_set_reset(&ins->parent, Instrument_reset);
    Device_set_mix_rate_changer(&ins->parent, Instrument_set_mix_rate);
    Device_set_buffer_size_changer(&ins->parent, Instrument_set_buffer_size);
    Device_set_sync(&ins->parent, Instrument_sync);
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_SEND, 0);
    ins->gens = new_Gen_table(KQT_GENERATORS_MAX);
    ins->effects = new_Effect_table(KQT_INST_EFFECTS_MAX);
    if (ins->gens == NULL || ins->effects == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }

//    ins->default_force = INS_DEFAULT_FORCE;
    ins->params.force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scale_index = INS_DEFAULT_SCALE_INDEX;

    return ins;
}


bool Instrument_parse_header(Instrument* ins, char* str, Read_state* state)
{
    assert(ins != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    double global_force = 1;
    double default_force = INS_DEFAULT_FORCE;
    double force_variation = INS_DEFAULT_FORCE_VAR;
#if 0
    bool pitch_lock_enabled = false;
    double pitch_lock_cents = 0;
#endif
    int64_t scale_index = INS_DEFAULT_SCALE_INDEX;
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
            bool expect_key = true;
            while (expect_key)
            {
                char key[128] = { '\0' };
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (string_eq(key, "force"))
                {
                    str = read_double(str, &default_force, state);
                }
                else if (string_eq(key, "force_variation"))
                {
                    str = read_double(str, &force_variation, state);
                }
                else if (string_eq(key, "global_force"))
                {
                    str = read_double(str, &global_force, state);
                }
#if 0
                else if (string_eq(key, "pitch_lock"))
                {
                    str = read_bool(str, &pitch_lock_enabled, state);
                }
                else if (string_eq(key, "pitch_lock_cents"))
                {
                    str = read_double(str, &pitch_lock_cents, state);
                }
#endif
                else if (string_eq(key, "scale"))
                {
                    str = read_int(str, &scale_index, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (scale_index < -1 || scale_index >= KQT_SCALES_MAX)
                    {
                        Read_state_set_error(state,
                                 "Invalid scale index: %" PRId64, scale_index);
                        return false;
                    }
                }
                else
                {
                    Read_state_set_error(state,
                             "Unsupported field in instrument information: %s", key);
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
    ins->params.force = default_force;
    ins->params.force_variation = force_variation;
    ins->params.global_force = exp2(global_force / 6);
#if 0
    ins->params.pitch_lock_enabled = pitch_lock_enabled;
    ins->params.pitch_lock_cents = pitch_lock_cents;
    ins->params.pitch_lock_freq = exp2(ins->params.pitch_lock_cents / 1200.0) * 440;
#endif
    return true;
}


bool Instrument_parse_value(
        Instrument* ins,
        const char* subkey,
        char* str,
        Read_state* state)
{
    assert(ins != NULL);
    assert(subkey != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    int gen_index = -1;
    if ((gen_index = string_extract_index(subkey,
                             "p_pitch_lock_enabled_", 2, ".json")) >= 0 &&
            gen_index < KQT_GENERATORS_MAX)
    {
        read_bool(str, &ins->params.pitch_locks[gen_index].enabled, state);
        if (state->error)
        {
            return false;
        }
    }
    else if ((gen_index = string_extract_index(subkey,
                                  "p_pitch_lock_cents_", 2, ".json")) >= 0 &&
            gen_index < KQT_GENERATORS_MAX)
    {
        read_double(str, &ins->params.pitch_locks[gen_index].cents, state);
        if (state->error)
        {
            return false;
        }
        ins->params.pitch_locks[gen_index].freq =
                exp2(ins->params.pitch_locks[gen_index].cents / 1200.0) * 440;
    }
    return true;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


Generator* Instrument_get_gen(Instrument* ins,
                              int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return Gen_table_get_gen(ins->gens, index);
}


Gen_table* Instrument_get_gens(Instrument* ins)
{
    assert(ins != NULL);
    return ins->gens;
}


Effect* Instrument_get_effect(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(ins->effects != NULL);
    assert(index >= 0);
    assert(index < KQT_INST_EFFECTS_MAX);
    return Effect_table_get(ins->effects, index);
}


Effect_table* Instrument_get_effects(Instrument* ins)
{
    assert(ins != NULL);
    assert(ins->effects != NULL);
    return ins->effects;
}


void Instrument_set_connections(Instrument* ins, Connections* graph)
{
    assert(ins != NULL);
    if (ins->connections != NULL)
    {
        del_Connections(ins->connections);
    }
    ins->connections = graph;
    return;
}


Connections* Instrument_get_connections(Instrument* ins)
{
    assert(ins != NULL);
    return ins->connections;
}


static void Instrument_reset(Device* device)
{
    assert(device != NULL);
    Instrument* ins = (Instrument*)device;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL)
        {
            Device_reset((Device*)gen);
        }
    }
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL)
        {
            Device_reset((Device*)eff);
        }
    }
    Instrument_params_reset(&ins->params);
    return;
}


static bool Instrument_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    Instrument* ins = (Instrument*)device;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL && !Device_set_mix_rate((Device*)gen, mix_rate))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL && !Device_set_mix_rate((Device*)eff, mix_rate))
        {
            return false;
        }
    }
    return true;
}


static bool Instrument_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    Instrument* ins = (Instrument*)device;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL && !Device_set_buffer_size((Device*)gen, size))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL && !Device_set_buffer_size((Device*)eff, size))
        {
            return false;
        }
    }
    return true;
}


static bool Instrument_sync(Device* device)
{
    assert(device != NULL);
    Instrument* ins = (Instrument*)device;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL && !Device_sync((Device*)gen))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL && !Device_sync((Device*)eff))
        {
            return false;
        }
    }
    return true;
}


void del_Instrument(Instrument* ins)
{
    if (ins == NULL)
    {
        return;
    }
    Instrument_params_uninit(&ins->params);
    del_Connections(ins->connections);
    del_Gen_table(ins->gens);
    del_Effect_table(ins->effects);
    Device_uninit(&ins->parent);
    memory_free(ins);
    return;
}


