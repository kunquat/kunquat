

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Processor.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_type.h>
#include <memory.h>
#include <player/Channel.h>
#include <player/Voice.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Device_state* Processor_create_state_plain(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Proc_state* proc_state = memory_alloc_item(Proc_state);
    if (proc_state == NULL)
        return NULL;

    if (!Proc_state_init(proc_state, device, audio_rate, audio_buffer_size))
    {
        memory_free(proc_state);
        return NULL;
    }

    return &proc_state->parent;
}


static Device_state* Processor_create_dstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    const Device_impl* dimpl = Device_get_impl(device);

    if ((dimpl != NULL) && (dimpl->create_pstate != NULL))
        return dimpl->create_pstate(device, audio_rate, audio_buffer_size);

    return Processor_create_state_plain(device, audio_rate, audio_buffer_size);
}


Processor* new_Processor(int index, const Au_params* au_params)
{
    rassert(index >= 0);
    rassert(index < KQT_PROCESSORS_MAX);
    rassert(au_params != NULL);

    Processor* proc = memory_alloc_item(Processor);
    if (proc == NULL)
        return NULL;

    if (!Device_init(&proc->parent, true))
    {
        memory_free(proc);
        return NULL;
    }

    //fprintf(stderr, "New Processor %p\n", (void*)proc);
    proc->index = index;
    proc->au_params = au_params;

    proc->enable_voice_support = false;
    proc->enable_signal_support = false;

    Device_set_state_creator(&proc->parent, Processor_create_dstate);

    return proc;
}


void Processor_set_voice_signals(Processor* proc, bool enabled)
{
    rassert(proc != NULL);
    proc->enable_voice_support = enabled;
    return;
}


bool Processor_get_voice_signals(const Processor* proc)
{
    rassert(proc != NULL);
    return proc->enable_voice_support;
}


/*
void Processor_set_signal_support(Processor* proc, bool enabled)
{
    rassert(proc != NULL);
    proc->enable_signal_support = enabled;
    return;
}


bool Processor_get_signal_support(const Processor* proc)
{
    rassert(proc != NULL);
    return proc->enable_signal_support;
}
// */


const Au_params* Processor_get_au_params(const Processor* proc)
{
    rassert(proc != NULL);
    return proc->au_params;
}


void del_Processor(Processor* proc)
{
    if (proc == NULL)
        return;

    Device_deinit(&proc->parent);
    memory_free(proc);

    return;
}


