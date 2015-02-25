

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
#include <stdint.h>
#include <math.h>

#include <debug/assert.h>
#include <devices/Device_params.h>
#include <devices/generators/Generator_common.h>
#include <devices/generators/Generator_debug.h>
#include <memory.h>
#include <player/Work_buffers.h>
#include <string/common.h>


typedef struct Generator_debug
{
    Device_impl parent;
    bool single_pulse;
} Generator_debug;


static bool Generator_debug_init(Device_impl* dimpl);

static Set_bool_func Generator_debug_set_single_pulse;

static Generator_mix_func Generator_debug_mix;

static void del_Generator_debug(Device_impl* gen_impl);


Device_impl* new_Generator_debug(Generator* gen)
{
    Generator_debug* debug = memory_alloc_item(Generator_debug);
    if (debug == NULL)
        return NULL;

    debug->parent.device = (Device*)gen;

    Device_impl_register_init(&debug->parent, Generator_debug_init);
    Device_impl_register_destroy(&debug->parent, del_Generator_debug);

    return &debug->parent;
}


static bool Generator_debug_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Generator_debug* debug = (Generator_debug*)dimpl;

    Generator* gen = (Generator*)debug->parent.device;
    gen->mix = Generator_debug_mix;

    if (!Device_impl_register_set_bool(
                &debug->parent,
                "p_b_single_pulse.json",
                false,
                Generator_debug_set_single_pulse,
                NULL))
        return false;

    debug->single_pulse = false;

    return true;
}


static uint32_t Generator_debug_mix(
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
    //assert(string_eq(gen->type, "debug"));
    assert(gen_state != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)ins_state;
    (void)wbs;
    (void)tempo;

    const Work_buffer* wb_actual_pitches = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    const float* actual_pitches = Work_buffer_get_contents(wb_actual_pitches) + 1;
    const float* actual_forces = Work_buffer_get_contents(wb_actual_forces) + 1;

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
    float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

    Generator_debug* debug = (Generator_debug*)gen->parent.dimpl;
    if (debug->single_pulse)
    {
        if (offset < nframes)
        {
            const float val = 1.0 * actual_forces[offset];
            audio_l[offset] = val;
            audio_r[offset] = val;
            vstate->active = false;
            return offset + 1;
        }
        return offset;
    }

    for (uint32_t i = offset; i < nframes; ++i)
    {
        const float actual_pitch = actual_pitches[i];
        const float actual_force = actual_forces[i];

        double vals[KQT_BUFFERS_MAX] = { 0 };

        if (vstate->rel_pos == 0)
        {
            vals[0] = vals[1] = 1.0;
            vstate->rel_pos = 1;
        }
        else
        {
            vals[0] = vals[1] = 0.5;
        }

        if (!vstate->note_on)
        {
            vals[0] = -vals[0];
            vals[1] = -vals[1];
        }

        vals[0] *= actual_force;
        vals[1] *= actual_force;

        audio_l[i] = vals[0];
        audio_r[i] = vals[1];

        vstate->rel_pos_rem += actual_pitch / freq;

        if (!vstate->note_on)
        {
            vstate->noff_pos_rem += actual_pitch / freq;
            if (vstate->noff_pos_rem >= 2)
            {
                vstate->active = false;
                return i + 1;
            }
        }

        if (vstate->rel_pos_rem >= 1)
        {
            ++vstate->pos;
            if (vstate->pos >= 10)
            {
                vstate->active = false;
                return i + 1;
            }
            vstate->rel_pos = 0;
            vstate->rel_pos_rem -= floor(vstate->rel_pos_rem);
        }
    }

    return nframes;
}


static bool Generator_debug_set_single_pulse(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Generator_debug* debug = (Generator_debug*)dimpl;
    debug->single_pulse = value;

    return true;
}


static void del_Generator_debug(Device_impl* gen_impl)
{
    if (gen_impl == NULL)
        return;

    Generator_debug* debug = (Generator_debug*)gen_impl;
    memory_free(debug);

    return;
}


