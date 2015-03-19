

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
#include <devices/Processor.h>
#include <devices/processors/Proc_pulse.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Voice_state_pulse.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>


typedef struct Pulse_state
{
    Proc_state parent;
    double pulse_width;
} Pulse_state;


typedef struct Proc_pulse
{
    Device_impl parent;
} Proc_pulse;


static bool Proc_pulse_init(Device_impl* dimpl);

static Device_state* Proc_pulse_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Proc_pulse_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static Proc_process_vstate_func Proc_pulse_process_vstate;

static void del_Proc_pulse(Device_impl* dimpl);


Device_impl* new_Proc_pulse(Processor* proc)
{
    Proc_pulse* pulse = memory_alloc_item(Proc_pulse);
    if (pulse == NULL)
        return NULL;

    pulse->parent.device = (Device*)proc;

    Device_impl_register_init(&pulse->parent, Proc_pulse_init);
    Device_impl_register_destroy(&pulse->parent, del_Proc_pulse);

    return &pulse->parent;
}


static bool Proc_pulse_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_pulse* pulse = (Proc_pulse*)dimpl;

    Processor* proc = (Processor*)pulse->parent.device;
    proc->init_vstate = Proc_pulse_init_vstate;
    proc->process_vstate = Proc_pulse_process_vstate;

    Device_set_state_creator(pulse->parent.device, Proc_pulse_create_state);

    return true;
}


const char* Proc_pulse_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    //assert(string_eq(proc->type, "pulse"));
    assert(property_type != NULL);
    (void)proc;

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_pulse));

        return size_str;
    }
    else if (string_eq(property_type, "proc_state_vars"))
    {
        static const char* vars_str = "[[\"F\", \"w\"]]"; // pulse width
        return vars_str;
    }

    return NULL;
}


static Device_state* Proc_pulse_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Pulse_state* pulse_state = memory_alloc_item(Pulse_state);
    if (pulse_state == NULL)
        return NULL;

    Proc_state_init(&pulse_state->parent, device, audio_rate, audio_buffer_size);
    pulse_state->pulse_width = 0.5;

    return &pulse_state->parent.parent;
}


static void Proc_pulse_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    //assert(string_eq(proc->type, "pulse"));
    (void)proc;
    assert(proc_state != NULL);
    assert(vstate != NULL);

    Voice_state_pulse* pulse_vstate = (Voice_state_pulse*)vstate;
    const Pulse_state* pulse_state = (const Pulse_state*)proc_state;
    pulse_vstate->phase = 0;
    pulse_vstate->pulse_width = pulse_state->pulse_width;

    return;
}


double pulse(double phase, double pulse_width)
{
    return (phase < pulse_width) ? 1.0 : -1.0;
}


uint32_t Proc_pulse_process_vstate(
        const Processor* proc,
        Proc_state* proc_state,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    //assert(string_eq(proc->type, "pulse"));
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    (void)proc_state;
    (void)au_state;
    (void)tempo;

//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_pulse* pulse_vstate = (Voice_state_pulse*)vstate;

    if (vstate->note_on)
    {
        const double* pulse_width_arg = Channel_proc_state_get_float(
                vstate->cpstate, "w");
        if (pulse_width_arg != NULL)
            pulse_vstate->pulse_width = *pulse_width_arg;
        else
            pulse_vstate->pulse_width = 0.5;
    }

    const float* actual_pitches = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const float* actual_forces = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_pitch = actual_pitches[i];
        const float actual_force = actual_forces[i];

        double val = pulse(pulse_vstate->phase, pulse_vstate->pulse_width) / 6;

        audio_l[i] = val * actual_force;

        pulse_vstate->phase += actual_pitch / audio_rate;
        if (pulse_vstate->phase >= 1)
            pulse_vstate->phase -= floor(pulse_vstate->phase);
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);

    Proc_ramp_attack(proc, vstate, wbs, 1, audio_rate, buf_start, buf_stop);

    Work_buffer_copy(wb_audio_r, wb_audio_l, buf_start, buf_stop);

    vstate->pos = 1; // XXX: hackish

    return buf_stop;
}


void del_Proc_pulse(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_pulse* pulse = (Proc_pulse*)dimpl;
    memory_free(pulse);

    return;
}


