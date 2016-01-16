

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_stream.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Stream_state.h>

#include <stdlib.h>


static void del_Proc_stream(Device_impl* dimpl);


Device_impl* new_Proc_stream(void)
{
    Proc_stream* stream = memory_alloc_item(Proc_stream);
    if (stream == NULL)
        return NULL;

    if (!Device_impl_init(&stream->parent, del_Proc_stream))
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

    stream->parent.create_pstate = new_Stream_pstate;
    stream->parent.get_vstate_size = Stream_vstate_get_size;
    stream->parent.init_vstate = Stream_vstate_init;

    return &stream->parent;
}


static void del_Proc_stream(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_stream* stream = (Proc_stream*)dimpl;
    memory_free(stream);

    return;
}


