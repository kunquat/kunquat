

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
#include <Env_var.h>
#include <Set_binding.h>
#include <Set_map.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


struct Set_map
{
    AAtree* map;
    Set_binding* cur;
};


static const int to_active[] =
{
    [ENV_VAR_NONE]    = -1,
    [ENV_VAR_BOOL]    = ACTIVE_TYPE_BOOL,
    [ENV_VAR_INT]     = ACTIVE_TYPE_INT,
    [ENV_VAR_FLOAT]   = ACTIVE_TYPE_FLOAT,
    [ENV_VAR_REAL]    = -1,
    [ENV_VAR_RELTIME] = ACTIVE_TYPE_TIMESTAMP,
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
    map->cur = NULL;
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


bool Set_map_get_first(Set_map* map,
                       Active_names* names,
                       char* src_event,
                       char* dest_event,
                       int dest_size,
                       int* channel)
{
    assert(map != NULL);
    assert(names != NULL);
    assert(src_event != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    assert(channel != NULL);
    Read_state* state = READ_STATE_AUTO;
    char* str = read_const_char(src_event, '[', state);
    char type_str[EVENT_NAME_MAX + 1] = "";
    str = read_string(str, type_str, EVENT_NAME_MAX, state);
    str = read_const_char(str, ',', state);
    str = read_const_char(str, '[', state);
    assert(!state->error);
    assert(strlen(type_str) == 3);
    Env_var_type type = ENV_VAR_NONE;
    switch (type_str[2])
    {
        case 'B':
        {
            type = ENV_VAR_BOOL;
        } break;
        case 'I':
        {
            type = ENV_VAR_INT;
        } break;
        case 'F':
        {
            type = ENV_VAR_FLOAT;
        } break;
        case 'T':
        {
            type = ENV_VAR_RELTIME;
        } break;
        default:
            assert(false);
    }
    char* var_name = Active_names_get(names, ACTIVE_CAT_ENV, to_active[type]);
    map->cur = AAtree_get_exact(map->map, var_name);
    if (map->cur == NULL)
    {
        return false;
    }
    return Set_binding_get_first(map->cur, str, dest_event, dest_size,
                                 channel);
}


bool Set_map_get_next(Set_map* map,
                      char* dest_event,
                      int dest_size,
                      int* channel)
{
    assert(map != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    assert(channel != NULL);
    if (map->cur == NULL)
    {
        return false;
    }
    return Set_binding_get_next(map->cur, dest_event, dest_size, channel);
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


