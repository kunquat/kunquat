

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>
#include <player/Channel_cv_state.h>


typedef struct Entry
{
    int au_index;
    char var_name[KQT_VAR_NAME_MAX];
    Value value;
    bool is_empty;
} Entry;


#define ENTRY_AUTO (&(Entry){ .au_index = 0, .var_name = "", .is_empty = true })


static int Entry_cmp(const Entry* e1, const Entry* e2)
{
    if (e1->au_index < e2->au_index)
        return -1;
    else if (e1->au_index > e2->au_index)
        return 1;

    const int name_diff = strcmp(e1->var_name, e2->var_name);
    if (name_diff != 0)
        return name_diff;

    if (e1->value.type < e2->value.type)
        return -1;
    else if (e1->value.type > e2->value.type)
        return 1;

    return 0;
}


static Entry* Entry_init(
        Entry* entry, int au_index, const char* var_name, Value_type var_type)
{
    assert(entry != NULL);
    assert(au_index >= 0);
    assert(au_index < KQT_AUDIO_UNITS_MAX);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    entry->au_index = au_index;
    strcpy(entry->var_name, var_name);

    if (var_type == VALUE_TYPE_BOOL)
    {
        entry->value.type = var_type;
        entry->value.value.bool_type = false;
    }
    else
    {
        Value* zero = VALUE_AUTO;
        zero->type = VALUE_TYPE_INT;
        zero->value.int_type = 0;
        Value_convert(&entry->value, zero, var_type);
    }

    entry->is_empty = true;

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
            (int (*)(const void*, const void*))Entry_cmp,
            (void (*)(void*))memory_free);
    if (state->tree == NULL)
    {
        del_Channel_cv_state(state);
        return NULL;
    }

    return state;
}


bool Channel_cv_state_add_entry(
        Channel_cv_state* state,
        int au_index,
        const char* var_name,
        Value_type var_type)
{
    assert(state != NULL);
    assert(au_index >= 0);
    assert(au_index < KQT_AUDIO_UNITS_MAX);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* entry = Entry_init(ENTRY_AUTO, au_index, var_name, var_type);

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


void Channel_cv_state_remove_entries(Channel_cv_state* state, int au_index)
{
    assert(state != NULL);
    assert(au_index >= 0);
    assert(au_index < KQT_AUDIO_UNITS_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, au_index, "", VALUE_TYPE_BOOL);

    const Entry* found_entry = AAtree_get_at_least(state->tree, key);
    while ((found_entry != NULL) && (found_entry->au_index == key->au_index))
    {
        memory_free(AAtree_remove(state->tree, found_entry));
        found_entry = AAtree_get_at_least(state->tree, key);
    }

    return;
}


bool Channel_cv_state_set_value(
        Channel_cv_state* state, int au_index, const char* var_name, const Value* value)
{
    assert(state != NULL);
    assert(au_index >= 0);
    assert(au_index < KQT_AUDIO_UNITS_MAX);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert(value != NULL);
    assert((value->type == VALUE_TYPE_BOOL) ||
            (value->type == VALUE_TYPE_INT) ||
            (value->type == VALUE_TYPE_FLOAT) ||
            (value->type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, au_index, var_name, value->type);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Value_copy(&entry->value, value);
    entry->is_empty = false;

    return true;
}


const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state,
        int au_index,
        const char* var_name,
        Value_type var_type)
{
    assert(state != NULL);
    assert(au_index >= 0);
    assert(au_index < KQT_AUDIO_UNITS_MAX);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, au_index, var_name, var_type);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || entry->is_empty)
        return NULL;

    return &entry->value;
}


void Channel_cv_state_reset(Channel_cv_state* state)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, 0, "", VALUE_TYPE_BOOL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        entry->is_empty = true;
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


