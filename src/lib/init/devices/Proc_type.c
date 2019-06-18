

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
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
#include <init/devices/Proc_type_init.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Proc_type_info
{
    const char* type_name;
    Proc_cons* cons;
    bool needs_vstate_if_connected_to_mixed;
} Proc_type_info;


static Proc_type_info proc_type_infos[] =
{
#define PROC_TYPE(name) { #name, new_Proc_ ## name, false },
#include <init/devices/Proc_types.h>
    { NULL, NULL, false }
};


Proc_type Proc_type_get_from_string(const char* type_name)
{
    rassert(type_name != NULL);

    for (int i = 0; proc_type_infos[i].type_name != NULL; ++i)
    {
        if (string_eq(type_name, proc_type_infos[i].type_name))
            return (Proc_type)i;
    }

    return Proc_type_COUNT;
}


Proc_cons* Proc_type_get_cons(Proc_type type)
{
    rassert(type >= 0);
    rassert(type < Proc_type_COUNT);

    return proc_type_infos[type].cons;
}


void Proc_type_set_needs_vstate_if_connected_to_mixed(Proc_type type)
{
    rassert(type >= 0);
    rassert(type < Proc_type_COUNT);

    proc_type_infos[type].needs_vstate_if_connected_to_mixed = true;

    return;
}


bool Proc_type_needs_vstate_if_connected_to_mixed(Proc_type type)
{
    rassert(type >= 0);
    rassert(type < Proc_type_COUNT);

    return proc_type_infos[type].needs_vstate_if_connected_to_mixed;
}


