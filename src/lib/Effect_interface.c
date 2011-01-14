

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Device.h>
#include <Effect_interface.h>
#include <xassert.h>
#include <xmemory.h>


Effect_interface* new_Effect_interface(uint32_t buf_len,
                                       uint32_t mix_rate)
{
    assert(buf_len > 0);
    assert(mix_rate > 0);
    Effect_interface* ei = xalloc(Effect_interface);
    if (ei == NULL)
    {
        return NULL;
    }
    if (!Device_init(&ei->parent, buf_len, mix_rate))
    {
        del_Effect_interface(ei);
        return NULL;
    }
    return ei;
}


void del_Effect_interface(Effect_interface* ei)
{
    if (ei == NULL)
    {
        return;
    }
    Device_uninit(&ei->parent);
    xfree(ei);
    return;
}


