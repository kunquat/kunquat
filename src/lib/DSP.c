

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
#include <string.h>

#include <Device.h>
#include <DSP.h>
#include <DSP_conf.h>
#include <DSP_type.h>
#include <File_base.h>
#include <player/Device_states.h>
#include <string_common.h>
#include <xassert.h>


DSP* new_DSP(
        char* str,
        uint32_t buffer_size,
        uint32_t mix_rate,
        Read_state* state)
{
    assert(str != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(state != NULL);

    if (state->error)
        return NULL;

    char type[DSP_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, DSP_TYPE_LENGTH_MAX, state);
    if (state->error)
        return NULL;

    DSP_cons* cons = DSP_type_find_cons(type);
#if 0
    DSP* (*cons)(uint32_t, uint32_t) = NULL;
    for (int i = 0; dsp_types[i].type != NULL; ++i)
    {
        if (string_eq(type, dsp_types[i].type))
        {
            cons = dsp_types[i].cons;
            break;
        }
    }
#endif
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported DSP type: \"%s\"\n", type);
        return NULL;
    }

    DSP* dsp = cons(buffer_size, mix_rate);
    if (dsp == NULL)
        return NULL;

    //fprintf(stderr, "New DSP %p\n", (void*)dsp);
    strcpy(dsp->type, type);

    return dsp;
}


bool DSP_init(
        DSP* dsp,
        void (*destroy)(DSP*),
        void (*process)(
            Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double),
        uint32_t buffer_size,
        uint32_t mix_rate)
{
    assert(dsp != NULL);
    assert(destroy != NULL);
    assert(process != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    dsp->clear_history = NULL;
    dsp->destroy = destroy;
    dsp->conf = NULL;

    if (!Device_init(&dsp->parent, buffer_size, mix_rate))
        return false;

    Device_set_reset(&dsp->parent, DSP_reset);
    Device_set_process(&dsp->parent, process);

    return true;
}


void DSP_set_clear_history(DSP* dsp, void (*func)(DSP*))
{
    assert(dsp != NULL);
    assert(func != NULL);

    dsp->clear_history = func;

    return;
}


void DSP_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    DSP* dsp = (DSP*)device;
    Device_params_reset(dsp->conf->params);

    return;
}


void DSP_clear_history(DSP* dsp)
{
    assert(dsp != NULL);

    if (dsp->clear_history != NULL)
        dsp->clear_history(dsp);

    return;
}


void DSP_set_conf(DSP* dsp, DSP_conf* conf)
{
    assert(dsp != NULL);
    assert(conf != NULL);
    assert(dsp->conf == NULL || dsp->conf == conf);

    dsp->conf = conf;

    return;
}


void del_DSP(DSP* dsp)
{
    if (dsp == NULL)
        return;

    assert(dsp->destroy != NULL);
    Device_deinit(&dsp->parent);
    dsp->destroy(dsp);

    return;
}


