

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <debug/assert.h>
#include <devices/dsps/DSP_type.h>
#include <string/common.h>


struct DSP_type
{
    const char* type;
    DSP_cons* cons;
    DSP_property* property;
};


static const DSP_type dsp_types[] =
{
    { NULL, NULL, NULL }
};


DSP_cons* DSP_type_find_cons(const char* type)
{
    assert(type != NULL);

    for (int i = 0; dsp_types[i].type != NULL; ++i)
    {
        if (string_eq(type, dsp_types[i].type))
            return dsp_types[i].cons;
    }

    return NULL;
}


DSP_property* DSP_type_find_property(const char* type)
{
    assert(type != NULL);

    for (int i = 0; dsp_types[i].type != NULL; ++i)
    {
        if (string_eq(type, dsp_types[i].type))
            return dsp_types[i].property;
    }

    assert(false);
    return NULL;
}


