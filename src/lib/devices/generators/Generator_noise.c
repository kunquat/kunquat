

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2014
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <devices/Device_params.h>
#include <devices/generators/Generator_common.h>
#include <devices/generators/Generator_noise.h>
#include <devices/generators/Voice_state_noise.h>
#include <Generator.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


typedef struct Noise_state
{
    Gen_state parent;
    int order;
} Noise_state;


typedef struct Generator_noise
{
    Device_impl parent;
} Generator_noise;


static Device_state* Generator_noise_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void Generator_noise_init_vstate(
        const Generator* gen,
        const Gen_state* gen_state,
        Voice_state* vstate);

static uint32_t Generator_noise_mix(
        const Generator* gen,
        Gen_state* gen_state,
        Ins_state* ins_state,
        Voice_state* vstate,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo);

static void del_Generator_noise(Device_impl* gen_impl);


Device_impl* new_Generator_noise(Generator* gen)
{
    Generator_noise* noise = memory_alloc_item(Generator_noise);
    if (noise == NULL)
        return NULL;

    if (!Device_impl_init(&noise->parent, del_Generator_noise))
    {
        memory_free(noise);
        return NULL;
    }

    noise->parent.device = (Device*)gen;

    gen->init_vstate = Generator_noise_init_vstate;
    gen->mix = Generator_noise_mix;

    Device_set_state_creator(
            noise->parent.device,
            Generator_noise_create_state);

    return &noise->parent;
}


char* Generator_noise_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "noise"));
    assert(property_type != NULL);
    (void)gen;

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_noise));
        return size_str;
    }
    else if (string_eq(property_type, "gen_state_vars"))
    {
        static char* vars_str = "[[\"I\", \"o\"]]"; // noise order
        return vars_str;
    }

    return NULL;
}


static Device_state* Generator_noise_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Noise_state* noise_state = memory_alloc_item(Noise_state);
    if (noise_state == NULL)
        return NULL;

    Gen_state_init(&noise_state->parent, device, audio_rate, audio_buffer_size);
    noise_state->order = 0;

    return &noise_state->parent.parent;
}


static void Generator_noise_init_vstate(
        const Generator* gen,
        const Gen_state* gen_state,
        Voice_state* vstate)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "noise"));
    (void)gen;
    assert(gen_state != NULL);
    (void)gen_state;
    assert(vstate != NULL);

    Voice_state_noise* noise_vstate = (Voice_state_noise*)vstate;
    memset(noise_vstate->buf[0], 0, NOISE_MAX * sizeof(double));
    memset(noise_vstate->buf[1], 0, NOISE_MAX * sizeof(double));

    return;
}


static uint32_t Generator_noise_mix(
        const Generator* gen,
        Gen_state* gen_state,
        Ins_state* ins_state,
        Voice_state* vstate,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "noise"));
    assert(gen_state != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen_state, vstate, offset, bufs);
    Generator_common_check_active(gen, vstate, offset);
    Generator_common_check_relative_lengths(gen, vstate, freq, tempo);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);

    Noise_state* noise_state = (Noise_state*)gen_state;
    Voice_state_noise* noise_vstate = (Voice_state_noise*)vstate;

    if (vstate->note_on)
    {
        const int64_t* order_arg = Channel_gen_state_get_int(
                vstate->cgstate, "o");
        if (order_arg != NULL)
            noise_state->order = *order_arg;
        else
            noise_state->order = 0;
    }

    uint32_t mixed = offset;
    for (; mixed < nframes && vstate->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, vstate);
        double vals[KQT_BUFFERS_MAX] = { 0 };

        if(noise_state->order < 0)
        {
            vals[0] = dc_pole_filter(
                    -noise_state->order,
                    noise_vstate->buf[0],
                    Random_get_float_signal(vstate->rand_s));
            vals[1] = dc_pole_filter(
                    -noise_state->order,
                    noise_vstate->buf[1],
                    Random_get_float_signal(vstate->rand_s));
        }
        else
        {
            vals[0] = dc_zero_filter(
                    noise_state->order,
                    noise_vstate->buf[0],
                    Random_get_float_signal(vstate->rand_s));
            vals[1] = dc_zero_filter(
                    noise_state->order,
                    noise_vstate->buf[1],
                    Random_get_float_signal(vstate->rand_s));
        }

        Generator_common_handle_force(gen, ins_state, vstate, vals, 2, freq);
        Generator_common_handle_filter(gen, vstate, vals, 2, freq);
        Generator_common_ramp_attack(gen, vstate, vals, 2, freq);
        vstate->pos = 1; // XXX: hackish

//        Generator_common_handle_note_off(gen, vstate, vals, 2, freq);
        Generator_common_handle_panning(gen, vstate, vals, 2);
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


static void del_Generator_noise(Device_impl* gen_impl)
{
    if (gen_impl == NULL)
        return;

    //assert(string_eq(gen->type, "noise"));
    Generator_noise* noise = (Generator_noise*)gen_impl;
    memory_free(noise);

    return;
}


