

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
#include <player/Ins_state.h>
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


static Device_state* Instrument_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void Instrument_reset(Device* device, Device_states* dstates);

static bool Instrument_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t mix_rate);

static bool Instrument_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size);

static void Instrument_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo);

//static bool Instrument_sync(Device* device, Device_states* dstates);


Instrument* new_Instrument()
{
    Instrument* ins = memory_alloc_item(Instrument);
    if (ins == NULL)
        return NULL;

    //fprintf(stderr, "New Instrument %p\n", (void*)ins);
    ins->connections = NULL;
    ins->gens = NULL;
    ins->effects = NULL;

    if (!Device_init(&ins->parent))
    {
        memory_free(ins);
        return NULL;
    }
    if (Instrument_params_init(
                &ins->params,
                Device_get_id(&ins->parent)) == NULL)
    {
        Device_deinit(&ins->parent);
        memory_free(ins);
        return NULL;
    }

    Device_set_state_creator(&ins->parent, Instrument_create_state);
    Device_set_reset(&ins->parent, Instrument_reset);
    Device_register_set_audio_rate(&ins->parent, Instrument_set_audio_rate);
    Device_register_update_tempo(&ins->parent, Instrument_update_tempo);
    Device_register_set_buffer_size(&ins->parent, Instrument_set_buffer_size);
    //Device_set_sync(&ins->parent, Instrument_sync);
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


typedef struct ins_params
{
    double global_force;
    double default_force;
    double force_variation;
    int64_t scale_index;
} ins_params;

static bool read_ins_field(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    ins_params* p = userdata;

    if (string_eq(key, "force"))
        Streader_read_float(sr, &p->default_force);
    else if (string_eq(key, "force_variation"))
        Streader_read_float(sr, &p->force_variation);
    else if (string_eq(key, "global_force"))
        Streader_read_float(sr, &p->global_force);
#if 0
    else if (string_eq(key, "pitch_lock"))
        str = read_bool(str, &pitch_lock_enabled, state);
    else if (string_eq(key, "pitch_lock_cents"))
        str = read_double(str, &pitch_lock_cents, state);
#endif
    else if (string_eq(key, "scale"))
    {
        if (!Streader_read_int(sr, &p->scale_index))
            return false;

        if (p->scale_index < -1 || p->scale_index >= KQT_SCALES_MAX)
        {
            Streader_set_error(
                     sr, "Invalid scale index: %" PRId64, p->scale_index);
            return false;
        }
    }
    else
    {
        Streader_set_error(
                sr, "Unsupported field in instrument information: %s", key);
        return false;
    }

    return !Streader_is_error_set(sr);
}

bool Instrument_parse_header(Instrument* ins, Streader* sr)
{
    assert(ins != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    ins_params* p = &(ins_params)
    {
        .global_force = 1,
        .default_force = INS_DEFAULT_FORCE,
        .force_variation = INS_DEFAULT_FORCE_VAR,
#if 0
        bool pitch_lock_enabled = false;
        double pitch_lock_cents = 0;
#endif
        .scale_index = INS_DEFAULT_SCALE_INDEX,
    };

    if (Streader_has_data(sr) && !Streader_read_dict(sr, read_ins_field, p))
        return false;

    ins->params.force = p->default_force;
    ins->params.force_variation = p->force_variation;
    ins->params.global_force = exp2(p->global_force / 6);
#if 0
    ins->params.pitch_lock_enabled = pitch_lock_enabled;
    ins->params.pitch_lock_cents = pitch_lock_cents;
    ins->params.pitch_lock_freq = exp2(ins->params.pitch_lock_cents / 1200.0) * 440;
#endif

    return true;
}


bool Instrument_parse_value(Instrument* ins, const char* subkey, Streader* sr)
{
    assert(ins != NULL);
    assert(subkey != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    int gen_index = -1;
    if ((gen_index = string_extract_index(subkey,
                             "p_pitch_lock_enabled_", 2, ".json")) >= 0 &&
            gen_index < KQT_GENERATORS_MAX)
    {
        if (!Streader_read_bool(sr, &ins->params.pitch_locks[gen_index].enabled))
            return false;
    }
    else if ((gen_index = string_extract_index(subkey,
                                  "p_pitch_lock_cents_", 2, ".json")) >= 0 &&
            gen_index < KQT_GENERATORS_MAX)
    {
        if (!Streader_read_float(sr, &ins->params.pitch_locks[gen_index].cents))
            return false;

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


Generator* Instrument_get_gen(Instrument* ins, int index)
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
        del_Connections(ins->connections);
    ins->connections = graph;

    return;
}


Connections* Instrument_get_connections(Instrument* ins)
{
    assert(ins != NULL);
    return ins->connections;
}


static Device_state* Instrument_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Ins_state* is = memory_alloc_item(Ins_state);
    if (is == NULL)
        return NULL;

    Device_state_init(&is->parent, device, audio_rate, audio_buffer_size);
    Ins_state_reset(is);

    return &is->parent;
}


static void Instrument_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Instrument* ins = (Instrument*)device;

    // Reset generators
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL)
            Device_reset((Device*)gen, dstates);
    }

    // Reset effects
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL)
            Device_reset((Device*)eff, dstates);
    }

    // Reset instrument state
    Ins_state* ins_state = (Ins_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));
    Ins_state_reset(ins_state);

    return;
}


static bool Instrument_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t audio_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);

    Instrument* ins = (Instrument*)device;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL &&
                !Device_set_audio_rate((Device*)gen, dstates, audio_rate))
            return false;
    }

    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL &&
                !Device_set_audio_rate((Device*)eff, dstates, audio_rate))
            return false;
    }

    return true;
}


static void Instrument_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    const Instrument* ins = (const Instrument*)device;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL)
            Device_update_tempo((const Device*)gen, dstates, tempo);
    }

    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL)
            Device_update_tempo((const Device*)eff, dstates, tempo);
    }

    return;
}


static bool Instrument_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    const Instrument* ins = (const Instrument*)device;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL &&
                !Device_set_buffer_size((Device*)gen, dstates, size))
            return false;
    }

    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL &&
                !Device_set_buffer_size((Device*)eff, dstates, size))
            return false;
    }

    return true;
}


#if 0
static bool Instrument_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Instrument* ins = (Instrument*)device;

    // Sync generators
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL && !Device_sync((Device*)gen, dstates))
            return false;
    }

    // Sync effects
    for (int i = 0; i < KQT_INST_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(ins->effects, i);
        if (eff != NULL && !Device_sync((Device*)eff, dstates))
            return false;
    }

    return true;
}
#endif


void del_Instrument(Instrument* ins)
{
    if (ins == NULL)
        return;

    Instrument_params_deinit(&ins->params);
    del_Connections(ins->connections);
    del_Gen_table(ins->gens);
    del_Effect_table(ins->effects);
    Device_deinit(&ins->parent);
    memory_free(ins);

    return;
}


