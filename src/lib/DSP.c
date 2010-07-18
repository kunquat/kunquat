

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
#include <stdint.h>
#include <string.h>

#include <Device.h>
#include <DSP.h>
#include <DSP_conf.h>
#include <DSP_type.h>
#include <File_base.h>
#include <xassert.h>


DSP* new_DSP(char* str,
             uint32_t buffer_size,
             uint32_t mix_rate,
             Read_state* state)
{
    assert(str != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    char type[DSP_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, DSP_TYPE_LENGTH_MAX, state);
    if (state->error)
    {
        return NULL;
    }
    DSP* (*cons)(uint32_t, uint32_t) = NULL;
    for (int i = 0; dsp_types[i].type != NULL; ++i)
    {
        if (strcmp(type, dsp_types[i].type) == 0)
        {
            cons = dsp_types[i].cons;
            break;
        }
    }
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported DSP type: \"%s\"\n", type);
        return NULL;
    }
    DSP* dsp = cons(buffer_size, mix_rate);
    if (dsp == NULL)
    {
        return NULL;
    }
    strcpy(dsp->type, type);
    return dsp;
}


bool DSP_init(DSP* dsp,
              void (*destroy)(DSP*),
              void (*process)(Device*, uint32_t, uint32_t, uint32_t, double),
              uint32_t buffer_size,
              uint32_t mix_rate)
{
    assert(dsp != NULL);
    assert(destroy != NULL);
    assert(process != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    dsp->destroy = destroy;
    if (!Device_init(&dsp->parent, buffer_size, mix_rate))
    {
        return false;
    }
    Device_set_process(&dsp->parent, process);
    return true;
}


void DSP_set_conf(DSP* dsp, DSP_conf* conf)
{
    assert(dsp != NULL);
    assert(conf != NULL);
    dsp->conf = conf;
    return;
}


void del_DSP(DSP* dsp)
{
    assert(dsp != NULL);
    assert(dsp->destroy != NULL);
    Device_uninit(&dsp->parent);
    dsp->destroy(dsp);
    return;
}


