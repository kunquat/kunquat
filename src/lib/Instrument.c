

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
#include <Generator.h>
#include <Instrument.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


#if 0
typedef struct Gen_group
{
    Generator common_params;
    Generator* gen;
} Gen_group;
#endif


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
    Generator gen_conf[KQT_GENERATORS_MAX];
    Generator* gen[KQT_GENERATORS_MAX];
};


Instrument* new_Instrument(kqt_frame** bufs,
                           kqt_frame** vbufs,
                           kqt_frame** vbufs2,
                           int buf_count,
                           uint32_t buf_len,
                           Scale** scales,
                           Scale*** default_scale,
                           Random* random)
{
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs2 != NULL);
    assert(vbufs2[0] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
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
    if (Instrument_params_init(&ins->params,
                               bufs, vbufs, vbufs2,
                               buf_count, buf_len,
                               default_scale) == NULL)
    {
        xfree(ins);
        return NULL;
    }
    if (!Device_init(&ins->parent, buf_len))
    {
        Instrument_params_uninit(&ins->params);
        xfree(ins);
        return NULL;
    }
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&ins->parent, DEVICE_PORT_TYPE_SEND, 0);

    ins->default_force = INS_DEFAULT_FORCE;
    ins->params.force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scales = scales;
    ins->default_scale = default_scale;
    ins->scale_index = INS_DEFAULT_SCALE_INDEX;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ins->gen_conf[i].type_params = NULL;
        ins->gen[i] = NULL;
    }

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (!Generator_init(&ins->gen_conf[i]))
        {
            del_Instrument(ins);
            return NULL;
        }
        Generator_params* gen_params = new_Generator_params();
        if (gen_params == NULL)
        {
            Generator_uninit(&ins->gen_conf[i]);
            del_Instrument(ins);
            return NULL;
        }
        ins->gen_conf[i].type_params = gen_params;
        ins->gen_conf[i].random = random;
        ins->gen[i] = NULL;
    }
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


Generator* Instrument_get_common_gen_params(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return &ins->gen_conf[index];
}


void Instrument_set_gen(Instrument* ins,
                        int index,
                        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    if (ins->gen[index] != NULL && ins->gen[index] != gen)
    {
        del_Generator(ins->gen[index]);
    }
    ins->gen[index] = gen;
    return;
}


Generator* Instrument_get_gen(Instrument* ins,
                              int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return ins->gen[index];
}


void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    if (ins->gen[index] != NULL)
    {
        del_Generator(ins->gen[index]);
        ins->gen[index] = NULL;
    }
    return;
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
        if (ins->gen[i] != NULL)
        {
            Generator_mix(ins->gen[i],
                          &states[i], nframes, offset, freq, 120);
        }
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
    for (int i = 0; i < KQT_GENERATORS_MAX &&
                    ins->gen_conf[i].type_params != NULL; ++i)
    {
        if (ins->gen_conf[i].type_params != NULL)
        {
            del_Generator_params(ins->gen_conf[i].type_params);
        }
        Generator_uninit(&ins->gen_conf[i]);
        if (ins->gen[i] != NULL)
        {
            del_Generator(ins->gen[i]);
        }
    }
    if (ins->connections != NULL)
    {
        del_Connections(ins->connections);
    }
    Device_uninit(&ins->parent);
    xfree(ins);
    return;
}


