

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


#include <init/devices/processors/Proc_compress.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Compress_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static Device_impl_destroy_func del_Proc_compress;


Device_impl* new_Proc_compress(void)
{
    Proc_compress* compress = memory_alloc_item(Proc_compress);
    if (compress == NULL)
        return NULL;

    if (!Device_impl_init(&compress->parent, del_Proc_compress))
    {
        del_Device_impl(&compress->parent);
        return NULL;
    }

    compress->parent.create_pstate = new_Compress_pstate;
    compress->parent.get_vstate_size = Compress_vstate_get_size;
    compress->parent.init_vstate = Compress_vstate_init;

    return &compress->parent;
}


static void del_Proc_compress(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_compress* compress = (Proc_compress*)dimpl;
    memory_free(compress);

    return;
}


