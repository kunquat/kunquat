

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2015
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_noise.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_params.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <devices/processors/Voice_state_noise.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct Noise_state
{
    Proc_state parent;
    int order;
} Noise_state;


typedef struct Proc_noise
{
    Device_impl parent;
} Proc_noise;


static bool Proc_noise_init(Device_impl* dimpl);

static Device_state* Proc_noise_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Proc_noise_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static Proc_state_render_voice_func Noise_state_render_voice;

static void del_Proc_noise(Device_impl* proc_impl);


Device_impl* new_Proc_noise(Processor* proc)
{
    Proc_noise* noise = memory_alloc_item(Proc_noise);
    if (noise == NULL)
        return NULL;

    noise->parent.device = (Device*)proc;

    Device_impl_register_init(&noise->parent, Proc_noise_init);
    Device_impl_register_destroy(&noise->parent, del_Proc_noise);

    return &noise->parent;
}


static bool Proc_noise_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_noise* noise = (Proc_noise*)dimpl;

    Processor* proc = (Processor*)noise->parent.device;
    proc->init_vstate = Proc_noise_init_vstate;

    Device_set_state_creator(noise->parent.device, Proc_noise_create_state);

    return true;
}


const char* Proc_noise_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_noise));
        return size_str;
    }
    else if (string_eq(property_type, "proc_state_vars"))
    {
        static const char* vars_str = "[[\"I\", \"o\"]]"; // noise order
        return vars_str;
    }

    return NULL;
}


static Device_state* Proc_noise_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Noise_state* noise_state = memory_alloc_item(Noise_state);
    if (noise_state == NULL)
        return NULL;

    if (!Proc_state_init(&noise_state->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(noise_state);
        return NULL;
    }

    noise_state->parent.render_voice = Noise_state_render_voice;

    noise_state->order = 0;

    return &noise_state->parent.parent;
}


static void Proc_noise_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(vstate != NULL);

    Voice_state_noise* noise_vstate = (Voice_state_noise*)vstate;
    memset(noise_vstate->buf[0], 0, NOISE_MAX * sizeof(double));
    memset(noise_vstate->buf[1], 0, NOISE_MAX * sizeof(double));

    return;
}


static int32_t Noise_state_render_voice(
        Proc_state* proc_state,
        Voice_state* vstate,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(proc_state != NULL);
    assert(vstate != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);

    Noise_state* noise_state = (Noise_state*)proc_state;
    Voice_state_noise* noise_vstate = (Voice_state_noise*)vstate;

    if (vstate->note_on)
    {
        const int64_t* order_arg = Channel_proc_state_get_int(
                vstate->cpstate, "o");
        if (order_arg != NULL)
            noise_state->order = *order_arg;
        else
            noise_state->order = 0;
    }

    // Get actual forces
    const Cond_work_buffer* actual_forces = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_FORCES),
            1,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE));

    // Get output buffer for writing
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);
    float* audio_l = Audio_buffer_get_buffer(out_buffer, 0);
    float* audio_r = Audio_buffer_get_buffer(out_buffer, 1);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_force = Cond_work_buffer_get_value(actual_forces, i);

        double vals[KQT_BUFFERS_MAX] = { 0 };

        if (noise_state->order < 0)
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

        audio_l[i] = vals[0] * actual_force;
        audio_r[i] = vals[1] * actual_force;
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;
    Proc_ramp_attack(proc, vstate, out_buffer, 2, audio_rate, buf_start, buf_stop);

    vstate->pos = 1; // XXX: hackish

//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return buf_stop;
}


static void del_Proc_noise(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_noise* noise = (Proc_noise*)dimpl;
    memory_free(noise);

    return;
}


