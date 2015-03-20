

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
#include <string.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Channel_proc_state.h>
#include <string/common.h>
#include <Value.h>


typedef struct Entry
{
    char key[KQT_KEY_LENGTH_MAX];
    Value value;
    bool is_empty;
} Entry;


#define ENTRY_AUTO (&(Entry){ .key = "", .is_empty = true })


int Entry_cmp(const Entry* e1, const Entry* e2)
{
    const int diff = strcmp(e1->key, e2->key);
    if (diff != 0)
        return diff;

    if (e1->value.type < e2->value.type)
        return -1;
    else if (e1->value.type > e2->value.type)
        return 1;

    return 0;
}


struct Channel_proc_state
{
    AAtree* tree;
};


Channel_proc_state* new_Channel_proc_state(void)
{
    Channel_proc_state* state = memory_alloc_item(Channel_proc_state);
    if (state == NULL)
        return NULL;

    state->tree = new_AAtree(
            (int (*)(const void*, const void*))Entry_cmp,
            (void (*)(void*))memory_free);
    if (state->tree == NULL)
    {
        del_Channel_proc_state(state);
        return NULL;
    }

    return state;
}


static bool read_entry(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);
    assert(userdata != NULL);

    Channel_proc_state* cpstate = userdata;

    // Read data
    char type[4] = "";
    char name[KQT_KEY_LENGTH_MAX + 1] = "";

    if (!Streader_readf(sr, "[%s,%s]", 4, type, KQT_KEY_LENGTH_MAX + 1, name))
        return false;

    // Build entry
    Entry* entry = ENTRY_AUTO;

    if (string_eq(type, "B"))
    {
        entry->value.type = VALUE_TYPE_BOOL;
        entry->value.value.bool_type = false;
    }
    else if (string_eq(type, "F"))
    {
        entry->value.type = VALUE_TYPE_FLOAT;
        entry->value.value.float_type = 0.0;
    }
    else if (string_eq(type, "I"))
    {
        entry->value.type = VALUE_TYPE_INT;
        entry->value.value.int_type = 0;
    }
    else if (string_eq(type, "T"))
    {
        entry->value.type = VALUE_TYPE_TSTAMP;
        Tstamp_init(&entry->value.value.Tstamp_type);
    }
    else
    {
        Streader_set_error(
                sr, "Invalid processor state variable type: %s", type);
        return false;
    }

    if (strlen(name) >= KQT_KEY_LENGTH_MAX)
    {
        Streader_set_error(
                sr, "Processor state variable name is too long: %s", name);
        return false;
    }

    if (!AAtree_contains(cpstate->tree, entry))
    {
        // Allocate new entry
        Entry* new_entry = memory_alloc_item(Entry);
        if (new_entry == NULL)
            return false;

        *new_entry = *entry;
        if (!AAtree_ins(cpstate->tree, new_entry))
        {
            memory_free(new_entry);
            Streader_set_memory_error(
                    sr,
                    "Could not allocate memory for channel-specific"
                        " generator state");
            return false;
        }
    }

    return true;
}

bool Channel_proc_state_alloc_keys(Channel_proc_state* cpstate, Streader* sr)
{
    assert(cpstate != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Streader_has_data(sr))
        return true;

    return Streader_read_list(sr, read_entry, cpstate);
}


bool Channel_proc_state_modify_value(
        Channel_proc_state* cpstate,
        const char* key,
        const Value* value)
{
    assert(cpstate != NULL);
    assert(key != NULL);
    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    assert(value != NULL);
    assert(value->type != VALUE_TYPE_NONE);

    Entry* entry_key = ENTRY_AUTO;
    strcpy(entry_key->key, key);
    entry_key->value.type = value->type;

    Entry* entry = AAtree_get_exact(cpstate->tree, entry_key);
    if (entry == NULL)
        return false;

    Value_copy(&entry->value, value);
    entry->is_empty = false;

    return true;
}


const bool* Channel_proc_state_get_bool(
        const Channel_proc_state* cpstate,
        const char* key)
{
    assert(cpstate != NULL);
    assert(key != NULL);

    Entry* entry_key = ENTRY_AUTO;
    strcpy(entry_key->key, key);
    entry_key->value.type = VALUE_TYPE_BOOL;

    const Entry* entry = AAtree_get_exact(cpstate->tree, entry_key);
    if (entry == NULL || entry->is_empty)
        return NULL;

    return &entry->value.value.bool_type;
}


const int64_t* Channel_proc_state_get_int(
        const Channel_proc_state* cpstate,
        const char* key)
{
    assert(cpstate != NULL);
    assert(key != NULL);

    Entry* entry_key = ENTRY_AUTO;
    strcpy(entry_key->key, key);
    entry_key->value.type = VALUE_TYPE_INT;

    const Entry* entry = AAtree_get_exact(cpstate->tree, entry_key);
    if (entry == NULL || entry->is_empty)
        return NULL;

    return &entry->value.value.int_type;
}


const double* Channel_proc_state_get_float(
        const Channel_proc_state* cpstate,
        const char* key)
{
    assert(cpstate != NULL);
    assert(key != NULL);

    Entry* entry_key = ENTRY_AUTO;
    strcpy(entry_key->key, key);
    entry_key->value.type = VALUE_TYPE_FLOAT;

    const Entry* entry = AAtree_get_exact(cpstate->tree, entry_key);
    if (entry == NULL || entry->is_empty)
        return NULL;

    return &entry->value.value.float_type;
}


const Tstamp* Channel_proc_state_get_tstamp(
        const Channel_proc_state* cpstate,
        const char* key)
{
    assert(cpstate != NULL);
    assert(key != NULL);

    Entry* entry_key = ENTRY_AUTO;
    strcpy(entry_key->key, key);
    entry_key->value.type = VALUE_TYPE_TSTAMP;

    const Entry* entry = AAtree_get_exact(cpstate->tree, entry_key);
    if (entry == NULL || entry->is_empty)
        return NULL;

    return &entry->value.value.Tstamp_type;
}


void Channel_proc_state_clear(Channel_proc_state* cpstate)
{
    assert(cpstate != NULL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, cpstate->tree);

    Entry* entry = AAiter_get_at_least(iter, ENTRY_AUTO);
    while (entry != NULL)
    {
        entry->is_empty = true;
        entry = AAiter_get_next(iter);
    }

    return;
}


void del_Channel_proc_state(Channel_proc_state* cpstate)
{
    if (cpstate == NULL)
        return;

    del_AAtree(cpstate->tree);
    memory_free(cpstate);

    return;
}


