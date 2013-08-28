

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
#include <memory.h>
#include <player/Device_states.h>
#include <string_common.h>
#include <xassert.h>


DSP* new_DSP(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    DSP* dsp = memory_alloc_item(DSP);
    if (dsp == NULL)
        return NULL;

    if (!Device_init(&dsp->parent, buffer_size, mix_rate))
    {
        memory_free(dsp);
        return NULL;
    }

    dsp->conf = NULL;

#if 0
    if (state->error)
        return NULL;

    char type[DSP_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, DSP_TYPE_LENGTH_MAX, state);
    if (state->error)
        return NULL;

    DSP_cons* cons = DSP_type_find_cons(type);
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported DSP type: \"%s\"\n", type);
        return NULL;
    }

    DSP* dsp = cons(buffer_size, mix_rate);
    if (dsp == NULL)
        return NULL;
#endif

    //fprintf(stderr, "New DSP %p\n", (void*)dsp);
    //strcpy(dsp->type, type);

    return dsp;
}


bool DSP_init(
        DSP* dsp,
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
    assert(process != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    dsp->clear_history = NULL;
    dsp->conf = NULL;

    if (!Device_init(&dsp->parent, buffer_size, mix_rate))
        return false;

    //Device_set_reset(&dsp->parent, DSP_reset);
    Device_set_process(&dsp->parent, process);

    return true;
}


void DSP_set_clear_history(DSP* dsp, void (*func)(DSP*, DSP_state*))
{
    assert(dsp != NULL);
    assert(func != NULL);

    dsp->clear_history = func;

    return;
}


#if 0
void DSP_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    DSP* dsp = (DSP*)device;
    Device_params_reset(dsp->conf->params);

    return;
}
#endif


void DSP_clear_history(DSP* dsp, DSP_state* dsp_state)
{
    assert(dsp != NULL);

    if (dsp->clear_history != NULL)
        dsp->clear_history(dsp, dsp_state);

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

    Device_deinit(&dsp->parent);
    memory_free(dsp);

    return;
}


