

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010
 *          Ossi Saresoja, Finland 2010
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <Generator.h>
#include <Generator_common.h>
#include <Generator_noise.h>
#include <Generator_params.h>
#include <Voice_state_noise.h>
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>


void Generator_noise_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_noise(Instrument_params* ins_params,
                               Generator_params* gen_params)
{
    assert(ins_params != NULL);
    assert(gen_params != NULL);
    Generator_noise* noise = xalloc(Generator_noise);
    if (noise == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&noise->parent))
    {
        xfree(noise);
        return NULL;
    }
//    noise->parent.parse = Generator_noise_parse;
    noise->parent.destroy = del_Generator_noise;
    noise->parent.type = GEN_TYPE_NOISE;
    noise->parent.init_state = Generator_noise_init_state;
    noise->parent.mix = Generator_noise_mix;
    noise->parent.ins_params = ins_params;
    noise->parent.type_params = gen_params;
    noise->order = 0;
    return &noise->parent;
}


#if 0
bool Generator_noise_has_subkey(const char* subkey)
{
    assert(subkey != NULL);
    return strcmp(subkey, "gen_noise/p_noise.json") == 0;
}
#endif


#if 0
bool Generator_noise_parse(Generator* gen,
                           const char* subkey,
                           void* data,
                           long length,
                           Read_state* state)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_NOISE);
    assert(subkey != NULL);
    assert(Generator_noise_has_subkey(subkey));
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Generator_noise* gen_noise = (Generator_noise*)gen;
    if (strcmp(subkey, "gen_noise/p_noise.json") == 0)
    {
        int64_t order = 0;
        char* str = data;
        if (str != NULL)
        {
            str = read_const_char(str, '{', state);
            str = read_const_string(str, "order", state);
            str = read_const_char(str, ':', state);
            str = read_int(str, &order, state);
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
        gen_noise->order = order;
        return true;
    }
    return false;
}
#endif


void Generator_noise_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
    assert(state != NULL);
    (void)gen;
    Voice_state_noise* noise_state = (Voice_state_noise*)state;
    noise_state->k = 0;
    for (int i = 0; i < 2; i++)
    {
        memset(noise_state->buf[i], 0, BINOM_MAX * sizeof(double)); 
        memset(noise_state->bufa[i], 0, BINOM_MAX * sizeof(double));
        memset(noise_state->bufb[i], 0, BINOM_MAX * sizeof(double));
    }
    return;
}


#define rand_u ((double)((rand() << 1) - RAND_MAX)/RAND_MAX)

uint32_t Generator_noise_mix(Generator* gen,
                             Voice_state* state,
                             uint32_t nframes,
                             uint32_t offset,
                             uint32_t freq,
                             double tempo,
                             int buf_count,
                             kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(tempo > 0);
    assert(buf_count > 0);
    (void)buf_count;
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Generator_noise* noise = (Generator_noise*)gen;
    Voice_state_noise* noise_state = (Voice_state_noise*)state;
    if (state->note_on)
    {
        int64_t* order_arg = Channel_gen_state_get_int(state->cgstate,
                                                      "order.jsoni");
        if (order_arg != NULL)
        {
            noise->order = *order_arg;
        }
        else
        {
            noise->order = 0;
        }
    }
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);
        
        double vals[KQT_BUFFERS_MAX] = { 0 };
        for (int i = 0; i < 2; ++i)
        {
            int k = noise_state->k;
            double* buf = noise_state->buf[i];
//          double* bufa = noise_state->bufa[i];
//          double* bufb = noise_state->bufb[i];
            double temp = rand_u;
            vals[i] = rand_u;
            if (noise->order > 0)
            {
                //int n =  noise->order;
                //iir_filter_strict(n, negbinom[n], buf, k, temp, vals[i]);
                //iir_filter_df1(n,    binom[n], negbinom[n], bufa, bufb, k, temp, vals[i]);
            }
            else if (noise->order < 0)
            {
                //int n = -noise->order;
                //fir_filter(n, negbinom[n], buf, k, temp, vals[i]);
                //iir_filter_df1(n, negbinom[n],    binom[n], bufa, bufb, k, temp, vals[i]);
            }
            noise_state->k = k;
            power_law_filter(noise->order, buf, temp, vals[i]);
        }
        Generator_common_handle_force(gen, state, vals, 2, freq);
        Generator_common_handle_filter(gen, state, vals, 2, freq);
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        state->pos = 1; // XXX: hackish
//        Generator_common_handle_note_off(gen, state, vals, 2, freq);
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
/*      if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return mixed;
}

#undef rand_u


void del_Generator_noise(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
    Generator_noise* noise = (Generator_noise*)gen;
    xfree(noise);
    return;
}


