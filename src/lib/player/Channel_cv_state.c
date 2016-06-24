

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Channel_cv_state.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <mathnum/Tstamp.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char var_name[KQT_VAR_NAME_MAX];

    Value value;
    bool is_set;
    bool carry;
} Entry;


#define ENTRY_AUTO (&(Entry){ .var_name = "", .is_set = false, .carry = false })


static Entry* Entry_init(Entry* entry, const char* var_name)
{
    assert(entry != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    strcpy(entry->var_name, var_name);

    entry->value.type = VALUE_TYPE_NONE;

    entry->is_set = false;
    entry->carry = false;

    return entry;
}


struct Channel_cv_state
{
    AAtree* tree;
};


Channel_cv_state* new_Channel_cv_state(void)
{
    Channel_cv_state* state = memory_alloc_item(Channel_cv_state);
    if (state == NULL)
        return NULL;

    state->tree = NULL;

    state->tree = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))memory_free);
    if (state->tree == NULL)
    {
        del_Channel_cv_state(state);
        return NULL;
    }

    return state;
}


bool Channel_cv_state_add_entry(Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* entry = Entry_init(ENTRY_AUTO, var_name);

    if (!AAtree_contains(state->tree, entry))
    {
        Entry* new_entry = memory_alloc_item(Entry);
        if (new_entry == NULL)
            return false;

        *new_entry = *entry;

        if (!AAtree_ins(state->tree, new_entry))
        {
            memory_free(new_entry);
            return false;
        }
    }

    return true;
}


bool Channel_cv_state_set_value(
        Channel_cv_state* state, const char* var_name, const Value* value)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert(value != NULL);
    assert(Value_type_is_realtime(value->type));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Value_copy(&entry->value, value);
    entry->is_set = true;

    return true;
}


const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !entry->is_set)
        return NULL;

    return &entry->value;
}


bool Channel_cv_state_set_carrying_enabled(
        Channel_cv_state* state, const char* var_name, bool enabled)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    entry->carry = enabled;

    return true;
}


bool Channel_cv_state_is_carrying_enabled(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    return entry->carry;
}


void Channel_cv_state_reset(Channel_cv_state* state)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, "");

    AAiter* iter = AAiter_init(AAITER_AUTO, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        entry->is_set = false;
        entry->carry = false;

        entry = AAiter_get_next(iter);
    }

    return;
}


void del_Channel_cv_state(Channel_cv_state* state)
{
    if (state == NULL)
        return;

    del_AAtree(state->tree);
    memory_free(state);

    return;
}


