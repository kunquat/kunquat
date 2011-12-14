

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

#include <AAtree.h>
#include <Set_binding.h>
#include <Set_map.h>
#include <xassert.h>
#include <xmemory.h>


struct Set_map
{
    AAtree* map;
};


Set_map* new_Set_map_from_string(char* str,
                                 Event_names* names,
                                 Read_state* state)
{
    assert(state != NULL);
    assert(names != NULL);
    Set_map* map = xalloc(Set_map);
    if (map == NULL)
    {
        return NULL;
    }
    map->map = new_AAtree((int (*)(const void*, const void*))strcmp,
                          (void (*)(void*))del_Set_binding);
    if (map->map == NULL)
    {
        del_Set_map(map);
        return NULL;
    }
    if (str == NULL)
    {
        return map;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Set_map(map);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return map;
    }
    Read_state_clear_error(state);
    bool expect_entry = true;
    while (expect_entry)
    {
        Set_binding* sb = new_Set_binding_from_string(&str, names, state);
        if (sb == NULL || !AAtree_ins(map->map, sb))
        {
            del_Set_binding(sb);
            del_Set_map(map);
            return NULL;
        }
        check_next(str, state, expect_entry);
    }
    return map;
}


void del_Set_map(Set_map* map)
{
    if (map == NULL)
    {
        return;
    }
    del_AAtree(map->map);
    xfree(map);
    return;
}


