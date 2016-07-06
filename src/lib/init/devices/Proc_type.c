

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
#include <init/devices/Proc_cons.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Proc_type
{
    const char* type;
    Proc_cons* cons;
} Proc_type;


static const Proc_type proc_types[] =
{
#define PROC_TYPE(name) { #name, new_Proc_ ## name },
#include <init/devices/Proc_types.h>
    { NULL, NULL }
};


Proc_cons* Proc_type_find_cons(const char* type)
{
    rassert(type != NULL);

    for (int i = 0; proc_types[i].type != NULL; ++i)
    {
        if (string_eq(type, proc_types[i].type))
            return proc_types[i].cons;
    }

    return NULL;
}


