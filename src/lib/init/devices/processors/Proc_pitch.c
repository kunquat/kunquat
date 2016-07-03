

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


#include <init/devices/processors/Proc_pitch.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <memory.h>
#include <player/devices/processors/Pitch_state.h>

#include <stdlib.h>


static void del_Proc_pitch(Device_impl* dimpl);


Device_impl* new_Proc_pitch(void)
{
    Proc_pitch* pitch = memory_alloc_item(Proc_pitch);
    if (pitch == NULL)
        return NULL;

    if (!Device_impl_init(&pitch->parent, del_Proc_pitch))
    {
        del_Device_impl(&pitch->parent);
        return NULL;
    }

    pitch->parent.get_vstate_size = Pitch_vstate_get_size;
    pitch->parent.init_vstate = Pitch_vstate_init;

    return &pitch->parent;
}


static void del_Proc_pitch(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_pitch* pitch = (Proc_pitch*)dimpl;
    memory_free(pitch);

    return;
}


