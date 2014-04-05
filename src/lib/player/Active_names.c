

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <module/Env_var.h>
#include <player/Active_names.h>


struct Active_names
{
    char names[ACTIVE_CAT_LAST][ACTIVE_TYPE_LAST][KQT_KEY_LENGTH_MAX];
};


Active_names* new_Active_names(void)
{
    Active_names* names = memory_alloc_item(Active_names);
    if (names == NULL)
    {
        return NULL;
    }
    Active_names_reset(names);
    return names;
}


bool Active_names_set(Active_names* names,
                      Active_cat cat,
                      Active_type type,
                      char* name)
{
    assert(names != NULL);
    assert(cat < ACTIVE_CAT_LAST);
    assert(type < ACTIVE_TYPE_LAST);
    assert(name != NULL);
    if (strlen(name) >= (cat == ACTIVE_CAT_ENV ? ENV_VAR_NAME_MAX :
                                                 KQT_KEY_LENGTH_MAX))
    {
        return false;
    }
    strcpy(names->names[cat][type], name);
    return true;
}


char* Active_names_get(Active_names* names, Active_cat cat, Active_type type)
{
    assert(names != NULL);
    assert(cat < ACTIVE_CAT_LAST);
    assert(type < ACTIVE_TYPE_LAST);
    return names->names[cat][type];
}


void Active_names_reset(Active_names* names)
{
    assert(names != NULL);
    memset(names->names, '\0', sizeof(names->names));
    return;
}


void del_Active_names(Active_names* names)
{
    if (names == NULL)
    {
        return;
    }
    memory_free(names);
    return;
}


