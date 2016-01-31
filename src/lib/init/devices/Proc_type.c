

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Proc_type.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_add.h>
#include <init/devices/processors/Proc_chorus.h>
#include <init/devices/processors/Proc_debug.h>
#include <init/devices/processors/Proc_envgen.h>
#include <init/devices/processors/Proc_filter.h>
#include <init/devices/processors/Proc_force.h>
#include <init/devices/processors/Proc_freeverb.h>
#include <init/devices/processors/Proc_gaincomp.h>
#include <init/devices/processors/Proc_noise.h>
#include <init/devices/processors/Proc_panning.h>
#include <init/devices/processors/Proc_pitch.h>
#include <init/devices/processors/Proc_ringmod.h>
#include <init/devices/processors/Proc_sample.h>
#include <init/devices/processors/Proc_stream.h>
#include <init/devices/processors/Proc_volume.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>


struct Proc_type
{
    const char* type;
    Proc_cons* cons;
};


static const Proc_type proc_types[] =
{
    { "debug", new_Proc_debug },
    { "add", new_Proc_add },
    { "chorus", new_Proc_chorus },
    { "envgen", new_Proc_envgen },
    { "filter", new_Proc_filter },
    { "force", new_Proc_force },
    { "freeverb", new_Proc_freeverb },
    { "gaincomp", new_Proc_gaincomp },
    { "noise", new_Proc_noise },
    { "panning", new_Proc_panning },
    { "pitch", new_Proc_pitch },
    { "ringmod", new_Proc_ringmod },
    { "sample", new_Proc_sample },
    { "stream", new_Proc_stream },
    { "volume", new_Proc_volume },
    { NULL, NULL }
};


Proc_cons* Proc_type_find_cons(const char* type)
{
    assert(type != NULL);

    for (int i = 0; proc_types[i].type != NULL; ++i)
    {
        if (string_eq(type, proc_types[i].type))
            return proc_types[i].cons;
    }

    return NULL;
}


