

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
#include <stdint.h>
#include <math.h>

#include <Generator_common.h>
#include <Generator_debug.h>
#include <Device_params.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


typedef struct Generator_debug
{
    Device_impl parent;
    bool single_pulse;
} Generator_debug;


static bool Generator_debug_update_key(Device* device, const char* key);

static uint32_t Generator_debug_mix(
        const Generator* gen,
        Gen_state* gen_state,
        Ins_state* ins_state,
        Voice_state* vstate,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo);

static void del_Generator_debug(Device_impl* gen_impl);


Device_impl* new_Generator_debug(Generator* gen, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    Generator_debug* debug = memory_alloc_item(Generator_debug);
    if (debug == NULL)
        return NULL;

    if (!Device_impl_init(&debug->parent, del_Generator_debug, mix_rate, buffer_size))
    {
        memory_free(debug);
        return NULL;
    }

    debug->parent.device = (Device*)gen;

    gen->mix = Generator_debug_mix;
#if 0
    if (!Generator_init(&debug->parent,
                        del_Generator_debug,
                        Generator_debug_mix,
                        NULL,
                        buffer_size,
                        mix_rate))
    {
        memory_free(debug);
        return NULL;
    }
#endif

    Device_set_update_key(debug->parent.device, Generator_debug_update_key);
    debug->single_pulse = false;

    return &debug->parent;
}


static uint32_t Generator_debug_mix(
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
    //assert(string_eq(gen->type, "debug"));
    assert(gen_state != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)tempo;

    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen_state, vstate, offset, bufs);

    if (!vstate->active)
        return offset;

    Generator_debug* debug = (Generator_debug*)gen->parent.dimpl;
    if (debug->single_pulse)
    {
        if (offset < nframes)
        {
            bufs[0][offset] += 1.0;
            bufs[1][offset] += 1.0;
            vstate->active = false;
            return offset + 1;
        }
        return offset;
    }

    for (uint32_t i = offset; i < nframes; ++i)
    {
        double val_l = 0;
        double val_r = 0;

        if (vstate->rel_pos == 0)
        {
            val_l = 1.0;
            val_r = 1.0;
            vstate->rel_pos = 1;
        }
        else
        {
            val_l = 0.5;
            val_r = 0.5;
        }

        if (!vstate->note_on)
        {
            val_l = -val_l;
            val_r = -val_r;
        }

        bufs[0][i] += val_l;
        bufs[1][i] += val_r;

        vstate->rel_pos_rem += vstate->pitch / freq;

        if (!vstate->note_on)
        {
            vstate->noff_pos_rem += vstate->pitch / freq;
            if (vstate->noff_pos_rem >= 2)
            {
                vstate->active = false;
                return i;
            }
        }

        if (vstate->rel_pos_rem >= 1)
        {
            ++vstate->pos;
            if (vstate->pos >= 10)
            {
                vstate->active = false;
                return i;
            }
            vstate->rel_pos = 0;
            vstate->rel_pos_rem -= floor(vstate->rel_pos_rem);
        }
    }

    return nframes;
}


static bool Generator_debug_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);

    Generator_debug* debug = (Generator_debug*)device->dimpl;
    Device_params* params = ((Generator*)device)->conf->params;
    if (string_eq(key, "p_single_pulse.jsonb"))
    {
        bool* value = Device_params_get_bool(params, key);
        if (value != NULL)
            debug->single_pulse = *value;
    }

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


