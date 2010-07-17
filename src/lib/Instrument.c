

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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Device.h>
#include <DSP.h>
#include <DSP_table.h>
#include <Gen_table.h>
#include <Generator.h>
#include <Instrument.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


struct Instrument
{
    Device parent;

    Connections* connections;

    double default_force;       ///< Default force.

    Scale** scales;             ///< The Scales of the Song.
    Scale*** default_scale;     ///< The default Scale of the Song.
    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Generators need.

//    Gen_group gens[KQT_GENERATORS_MAX]; ///< Generators.
//    Generator gen_conf[KQT_GENERATORS_MAX];
//    Generator* gens[KQT_GENERATORS_MAX];
    Gen_table* gens;

    DSP_table* dsps;
};


static bool Instrument_set_mix_rate(Device* device, uint32_t mix_rate);


static bool Instrument_set_buffer_size(Device* device, uint32_t size);


Instrument* new_Instrument(kqt_frame** bufs,
                           kqt_frame** vbufs,
                           kqt_frame** vbufs2,
                           int buf_count,
                           uint32_t buf_len,
                           uint32_t mix_rate,
                           Scale** scales,
                           Scale*** default_scale,
                           Random* random)
{
#if 0
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs2 != NULL);
    assert(vbufs2[0] != NULL);
#endif
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(mix_rate > 0);
    assert(scales != NULL);
    assert(default_scale != NULL);
    assert(*default_scale != NULL);
    assert(*default_scale >= &scales[0]);
    assert(*default_scale <= &scales[KQT_SCALES_MAX - 1]);
    assert(random != NULL);
    Instrument* ins = xalloc(Instrument);
    if (ins == NULL)
    {
        return NULL;
    }
    ins->connections = NULL;
    ins->gens = NULL;
    ins->dsps = NULL;
    
    if (Instrument_params_init(&ins->params,
                               bufs, vbufs, vbufs2,
                               buf_count, buf_len,
                               default_scale) == NULL)
    {
        xfree(ins);
        return NULL;
    }
    if (!Device_init(&ins->parent, buf_len, mix_rate))
    {
        Instrument_params_uninit(&ins->params);
        xfree(ins);
        return NULL;
    }
    Device_set_mix_rate_changer(&ins->parent, Instrument_set_mix_rate);
    Device_set_buffer_size_changer(&ins->parent, Instrument_set_buffer_size);
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_SEND, 0);
    ins->gens = new_Gen_table(KQT_GENERATORS_MAX);
    if (ins->gens == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->dsps = new_DSP_table(KQT_INSTRUMENT_DSPS_MAX);
    if (ins->dsps == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }

    ins->default_force = INS_DEFAULT_FORCE;
    ins->params.force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scales = scales;
    ins->default_scale = default_scale;
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
    double default_force = INS_DEFAULT_FORCE;
    double force_variation = INS_DEFAULT_FORCE_VAR;
    bool pitch_lock_enabled = false;
    double pitch_lock_cents = 0;
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
                if (strcmp(key, "force") == 0)
                {
                    str = read_double(str, &default_force, state);
                }
                else if (strcmp(key, "force_variation") == 0)
                {
                    str = read_double(str, &force_variation, state);
                }
                else if (strcmp(key, "pitch_lock") == 0)
                {
                    str = read_bool(str, &pitch_lock_enabled, state);
                }
                else if (strcmp(key, "pitch_lock_cents") == 0)
                {
                    str = read_double(str, &pitch_lock_cents, state);
                }
                else if (strcmp(key, "scale") == 0)
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
    ins->default_force = default_force;
    ins->params.force_variation = force_variation;
    ins->params.pitch_lock_enabled = pitch_lock_enabled;
    ins->params.pitch_lock_cents = pitch_lock_cents;
    ins->params.pitch_lock_freq = exp2(ins->params.pitch_lock_cents / 1200.0) * 440;
    Instrument_set_scale(ins, scale_index);
    return true;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


#if 0
Generator* Instrument_get_common_gen_params(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return &ins->gen_conf[index];
}
#endif


#if 0
void Instrument_set_gen(Instrument* ins,
                        int index,
                        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    if (ins->gens[index] != NULL && ins->gens[index] != gen)
    {
        del_Generator(ins->gens[index]);
    }
    ins->gens[index] = gen;
    return;
}
#endif


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


#if 0
void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    if (ins->gens[index] != NULL)
    {
        del_Generator(ins->gens[index]);
        ins->gens[index] = NULL;
    }
    return;
}
#endif


DSP* Instrument_get_dsp(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(ins->dsps != NULL);
    return DSP_table_get_dsp(ins->dsps, index);
}


DSP_table* Instrument_get_dsps(Instrument* ins)
{
    assert(ins != NULL);
    assert(ins->dsps != NULL);
    return ins->dsps;
}


void Instrument_set_scale(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= -1);
    assert(index < KQT_SCALES_MAX);
    if (index == -1 || true)
    {
        ins->params.scale = ins->default_scale;
    }
    else
    {
//        ins->params.scale = &ins->scales[index];
    }
    return;
}


void Instrument_set_connections(Instrument* ins, Connections* graph)
{
    assert(ins != NULL);
    assert(graph != NULL);
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


void Instrument_mix(Instrument* ins,
                    Voice_state* states,
                    uint32_t nframes,
                    uint32_t offset,
                    uint32_t freq)
{
    assert(ins != NULL);
    assert(states != NULL);
//  assert(nframes <= ins->buf_len);
    assert(freq > 0);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL)
        {
            Generator_mix(gen, &states[i], nframes, offset, freq, 120);
        }
    }
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
    for (int i = 0; i < KQT_INSTRUMENT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(ins->dsps, i);
        if (dsp != NULL && !Device_set_mix_rate((Device*)dsp, mix_rate))
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
    assert(size <= KQT_BUFFER_SIZE_MAX);
    Instrument* ins = (Instrument*)device;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator* gen = Gen_table_get_gen(ins->gens, i);
        if (gen != NULL && !Device_set_buffer_size((Device*)gen, size))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_INSTRUMENT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(ins->dsps, i);
        if (dsp != NULL && !Device_set_buffer_size((Device*)dsp, size))
        {
            return false;
        }
    }
    return true;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
#if 0
    for (int i = 0; i < KQT_GENERATORS_MAX &&
                    ins->gen_conf[i].type_params != NULL; ++i)
    {
        if (ins->gen_conf[i].type_params != NULL)
        {
            del_Device_params(ins->gen_conf[i].type_params);
        }
        Generator_uninit(&ins->gen_conf[i]);
        if (ins->gens[i] != NULL)
        {
            del_Generator(ins->gens[i]);
        }
    }
#endif
    if (ins->connections != NULL)
    {
        del_Connections(ins->connections);
    }
    if (ins->gens != NULL)
    {
        del_Gen_table(ins->gens);
    }
    if (ins->dsps != NULL)
    {
        del_DSP_table(ins->dsps);
    }
    Device_uninit(&ins->parent);
    xfree(ins);
    return;
}


