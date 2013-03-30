

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <AAtree.h>
#include <Channel_gen_state.h>
#include <Device_field.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


struct Channel_gen_state
{
    AAiter* iter;
    AAtree* tree;
};


Channel_gen_state* new_Channel_gen_state(void)
{
    Channel_gen_state* state = memory_alloc_item(Channel_gen_state);
    if (state == NULL)
    {
        return NULL;
    }
    state->iter = NULL;
    state->tree = new_AAtree((int (*)(const void*, const void*))strcmp,
                             (void (*)(void*))del_Device_field);
    if (state->tree == NULL)
    {
        del_Channel_gen_state(state);
        return NULL;
    }
    state->iter = new_AAiter(state->tree);
    if (state->iter == NULL)
    {
        del_Channel_gen_state(state);
        return NULL;
    }
    return state;
}


bool Channel_gen_state_set_key(Channel_gen_state* state, const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (AAtree_get_exact(state->tree, key) != NULL)
    {
        return true;
    }
    Device_field* field = new_Device_field(key, NULL);
    if (field == NULL)
    {
        return false;
    }
    if (!AAtree_ins(state->tree, field))
    {
        del_Device_field(field);
        return false;
    }
    return true;
}


bool Channel_gen_state_modify_value(Channel_gen_state* state,
                                    const char* key,
                                    void* data)
{
    assert(state != NULL);
    assert(key != NULL);
    assert(data != NULL);
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL)
    {
        return false;
    }
    return Device_field_modify(field, data);
}


bool* Channel_gen_state_get_bool(Channel_gen_state* state,
                                 const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonb"))
    {
        return NULL;
    }
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL || Device_field_get_empty(field))
    {
        return NULL;
    }
    return Device_field_get_bool(field);
}


int64_t* Channel_gen_state_get_int(Channel_gen_state* state,
                                   const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsoni"))
    {
        return NULL;
    }
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL || Device_field_get_empty(field))
    {
        return NULL;
    }
    return Device_field_get_int(field);
}


double* Channel_gen_state_get_float(Channel_gen_state* state,
                                    const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonf"))
    {
        return NULL;
    }
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL || Device_field_get_empty(field))
    {
        return NULL;
    }
    return Device_field_get_float(field);
}


Real* Channel_gen_state_get_real(Channel_gen_state* state,
                                 const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonr"))
    {
        return NULL;
    }
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL || Device_field_get_empty(field))
    {
        return NULL;
    }
    return Device_field_get_real(field);
}


Reltime* Channel_gen_state_get_reltime(Channel_gen_state* state,
                                       const char* key)
{
    assert(state != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsont"))
    {
        return NULL;
    }
    Device_field* field = AAtree_get_exact(state->tree, key);
    if (field == NULL || Device_field_get_empty(field))
    {
        return NULL;
    }
    return Device_field_get_reltime(field);
}


void Channel_gen_state_clear(Channel_gen_state* state)
{
    assert(state != NULL);
    Device_field* field = AAiter_get(state->iter, "");
    while (field != NULL)
    {
        Device_field_set_empty(field, true);
        field = AAiter_get_next(state->iter);
    }
    return;
}


void del_Channel_gen_state(Channel_gen_state* state)
{
    if (state == NULL)
    {
        return;
    }
    del_AAiter(state->iter);
    del_AAtree(state->tree);
    memory_free(state);
    return;
}


