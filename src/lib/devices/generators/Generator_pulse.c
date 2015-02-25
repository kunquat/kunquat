

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <math.h>
#include <string.h>

#include <debug/assert.h>
#include <devices/Device_params.h>
#include <devices/Generator.h>
#include <devices/generators/Generator_common.h>
#include <devices/generators/Generator_pulse.h>
#include <devices/generators/Voice_state_pulse.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>


typedef struct Pulse_state
{
    Gen_state parent;
    double pulse_width;
} Pulse_state;


typedef struct Generator_pulse
{
    Device_impl parent;
} Generator_pulse;


static bool Generator_pulse_init(Device_impl* dimpl);

static Device_state* Generator_pulse_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Generator_pulse_init_vstate(
        const Generator* gen, const Gen_state* gen_state, Voice_state* vstate);

static Generator_mix_func Generator_pulse_mix;

static void del_Generator_pulse(Device_impl* gen);


Device_impl* new_Generator_pulse(Generator* gen)
{
    Generator_pulse* pulse = memory_alloc_item(Generator_pulse);
    if (pulse == NULL)
        return NULL;

    pulse->parent.device = (Device*)gen;

    Device_impl_register_init(&pulse->parent, Generator_pulse_init);
    Device_impl_register_destroy(&pulse->parent, del_Generator_pulse);

    return &pulse->parent;
}


static bool Generator_pulse_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Generator_pulse* pulse = (Generator_pulse*)dimpl;

    Generator* gen = (Generator*)pulse->parent.device;
    gen->init_vstate = Generator_pulse_init_vstate;
    gen->mix = Generator_pulse_mix;

    Device_set_state_creator(
            pulse->parent.device,
            Generator_pulse_create_state);

    return true;
}


const char* Generator_pulse_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "pulse"));
    assert(property_type != NULL);
    (void)gen;

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_pulse));

        return size_str;
    }
    else if (string_eq(property_type, "gen_state_vars"))
    {
        static const char* vars_str = "[[\"F\", \"w\"]]"; // pulse width
        return vars_str;
    }

    return NULL;
}


static Device_state* Generator_pulse_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Pulse_state* pulse_state = memory_alloc_item(Pulse_state);
    if (pulse_state == NULL)
        return NULL;

    Gen_state_init(&pulse_state->parent, device, audio_rate, audio_buffer_size);
    pulse_state->pulse_width = 0.5;

    return &pulse_state->parent.parent;
}


static void Generator_pulse_init_vstate(
        const Generator* gen, const Gen_state* gen_state, Voice_state* vstate)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "pulse"));
    (void)gen;
    assert(gen_state != NULL);
    assert(vstate != NULL);

    Voice_state_pulse* pulse_vstate = (Voice_state_pulse*)vstate;
    const Pulse_state* pulse_state = (const Pulse_state*)gen_state;
    pulse_vstate->phase = 0;
    pulse_vstate->pulse_width = pulse_state->pulse_width;

    return;
}


double pulse(double phase, double pulse_width)
{
    return (phase < pulse_width) ? 1.0 : -1.0;
}


uint32_t Generator_pulse_mix(
        const Generator* gen,
        Gen_state* gen_state,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(gen != NULL);
    //assert(string_eq(gen->type, "pulse"));
    assert(gen_state != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen_state, vstate, offset, bufs);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_pulse* pulse_vstate = (Voice_state_pulse*)vstate;

    if (vstate->note_on)
    {
        const double* pulse_width_arg = Channel_gen_state_get_float(
                vstate->cgstate, "w");
        if (pulse_width_arg != NULL)
            pulse_vstate->pulse_width = *pulse_width_arg;
        else
            pulse_vstate->pulse_width = 0.5;
    }

    Generator_common_handle_pitch(gen, vstate, wbs, nframes, offset);

    const int32_t force_extent = Generator_common_handle_force(
            gen, ins_state, vstate, wbs, freq, nframes, offset);

    const bool force_ended = (force_extent < (int32_t)nframes);
    if (force_ended)
        nframes = force_extent;

    const Work_buffer* wb_pitch_params = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    const Work_buffer* wb_actual_pitches = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    float* pitch_params = Work_buffer_get_contents_mut(wb_pitch_params) + 1;
    float* actual_pitches = Work_buffer_get_contents_mut(wb_actual_pitches) + 1;
    float* actual_forces = Work_buffer_get_contents_mut(wb_actual_forces) + 1;

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
    float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

    uint32_t mixed = offset;
    for (; mixed < nframes && vstate->active; ++mixed)
    {
        //Generator_common_handle_pitch(gen, vstate);

        // Temp hack code
        vstate->pitch = pitch_params[mixed];
        vstate->actual_pitch = actual_pitches[mixed];
        vstate->prev_actual_pitch = actual_pitches[(int32_t)mixed - 1];
        vstate->actual_force = actual_forces[mixed];

        double val = pulse(pulse_vstate->phase, pulse_vstate->pulse_width) / 6;

        audio_l[mixed] = val * vstate->actual_force;

        //Generator_common_handle_force(gen, ins_state, vstate, vals, 1, freq);
        //Generator_common_handle_filter(gen, vstate, vals, 1, freq);
        //Generator_common_ramp_attack(gen, vstate, vals, 1, freq);

        pulse_vstate->phase += vstate->actual_pitch / freq;
        if (pulse_vstate->phase >= 1)
            pulse_vstate->phase -= floor(pulse_vstate->phase);

//        Generator_common_handle_note_off(gen, vstate, vals, 1, freq);
        //vals[1] = vals[0];
        //Generator_common_handle_panning(gen, vstate, vals, 2);
        //bufs[0][mixed] += vals[0];
        //bufs[1][mixed] += vals[1];
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);

    const int32_t release_limit = Generator_common_ramp_release(
            gen, ins_state, vstate, wbs, 1, freq, nframes, offset);
    if (release_limit < (int32_t)nframes)
        nframes = release_limit;
    const bool ramp_release_ended = (vstate->ramp_release >= 1);

    Work_buffer_copy(wb_audio_r, wb_audio_l, offset, nframes);

    Generator_common_handle_filter(gen, vstate, wbs, 2, freq, nframes, offset);
    Generator_common_ramp_attack(gen, vstate, wbs, 2, freq, nframes, offset);
    Generator_common_handle_panning(gen, vstate, wbs, nframes, offset);

    vstate->pos = 1; // XXX: hackish

    for (uint32_t i = offset; i < nframes; ++i)
        bufs[0][i] += audio_l[i];
    for (uint32_t i = offset; i < nframes; ++i)
        bufs[1][i] += audio_r[i];

    if (force_ended || ramp_release_ended)
        vstate->active = false;

    return mixed;
}


void del_Generator_pulse(Device_impl* gen_impl)
{
    if (gen_impl == NULL)
        return;

    //assert(string_eq(gen->type, "pulse"));
    Generator_pulse* pulse = (Generator_pulse*)gen_impl;
    memory_free(pulse);

    return;
}


