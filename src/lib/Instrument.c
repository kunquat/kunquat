

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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Generator.h>
#include <Instrument.h>
#include <File_base.h>

#include <xmemory.h>


typedef struct Gen_group
{
    Gen_type active_type;
    Generator common_params;
    Generator* types[GEN_TYPE_LAST];
} Gen_group;


struct Instrument
{
    double default_force;       ///< Default force.

    Scale** scales;             ///< The Scales of the Song.
    Scale*** default_scale;     ///< The default Scale of the Song.
    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Generators need.

    Gen_group gens[KQT_GENERATORS_MAX]; ///< Generators.
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
    if (Instrument_params_init(&ins->params,
                               bufs, vbufs, vbufs2,
                               buf_count, buf_len,
                               default_scale) == NULL)
    {
        xfree(ins);
        return NULL;
    }

    ins->default_force = INS_DEFAULT_FORCE;
    ins->params.force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scales = scales;
    ins->default_scale = default_scale;
    ins->scale_index = INS_DEFAULT_SCALE_INDEX;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ins->gens[i].active_type = GEN_TYPE_NONE;
        if (!Generator_init(&ins->gens[i].common_params))
        {
            for (int k = i - 1; k >= 0; --k)
            {
                Generator_uninit(&ins->gens[k].common_params);
            }
            xfree(ins);
            return NULL;
        }
        ins->gens[i].common_params.random = random;
        for (int k = 0; k < GEN_TYPE_LAST; ++k)
        {
            ins->gens[i].types[k] = NULL;
        }
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
    return &ins->gens[index].common_params;
}


#if 0
int Instrument_get_gen_count(Instrument* ins)
{
    assert(ins != NULL);
    return ins->gen_count;
}
#endif


void Instrument_set_gen(Instrument* ins,
                        int index,
                        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    Instrument_set_gen_of_type(ins, index, gen);
    ins->gens[index].active_type = Generator_get_type(gen);
    return;
}


Generator* Instrument_get_gen(Instrument* ins,
                              int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    return ins->gens[index].types[ins->gens[index].active_type];
}


void Instrument_set_gen_of_type(Instrument* ins,
                                int index,
                                Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(gen != NULL);
    Gen_type type = Generator_get_type(gen);
    if (ins->gens[index].types[type] != NULL &&
            ins->gens[index].types[type] != gen)
    {
        del_Generator(ins->gens[index].types[type]);
    }
    ins->gens[index].types[type] = gen;
    return;
}


Generator* Instrument_get_gen_of_type(Instrument* ins,
                                      int index,
                                      Gen_type type)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    return ins->gens[index].types[type];
}


void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_GENERATORS_MAX);
    Gen_type active_type = ins->gens[index].active_type;
    if (ins->gens[index].types[active_type] == NULL)
    {
        return;
    }
    del_Generator(ins->gens[index].types[active_type]);
    ins->gens[index].types[active_type] = NULL;
    ins->gens[index].active_type = GEN_TYPE_NONE;
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
        Gen_type active_type = ins->gens[i].active_type;
        if (ins->gens[i].types[active_type] != NULL)
        {
            Generator_mix(ins->gens[i].types[active_type],
                          &states[i], nframes, offset, freq, 120);
        }
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Generator_uninit(&ins->gens[i].common_params);
        for (int k = 0; k < GEN_TYPE_LAST; ++k)
        {
            if (ins->gens[i].types[k] != NULL)
            {
                del_Generator(ins->gens[i].types[k]);
            }
        }
    }
    xfree(ins);
    return;
}


