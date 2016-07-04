

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_expressions.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <init/devices/Param_proc_filter.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string/Streader.h>
#include <string/var_name.h>

#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char name[KQT_VAR_NAME_MAX];
    Param_proc_filter* filter;
} Entry;


#define ENTRY_AUTO (&(Entry){ .name = "", .filter = NULL })


static void del_Entry(Entry* entry)
{
    assert(entry != NULL);

    memory_free(entry->filter);
    memory_free(entry);

    return;
}


struct Au_expressions
{
    AAtree* entries;
};


static bool read_expressions(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    Au_expressions* ae = userdata;

    if (!is_valid_var_name(key))
    {
        Streader_set_error(sr, "Invalid expression name: %s", key);
        return false;
    }

    Entry* entry = memory_alloc_item(Entry);
    if (entry == NULL)
        return false;

    strcpy(entry->name, key);

    Param_proc_filter* filter = new_Param_proc_filter(sr);
    if (filter == NULL)
    {
        memory_free(entry);
        return false;
    }

    entry->filter = filter;

    if (!AAtree_ins(ae->entries, entry))
    {
        del_Param_proc_filter(filter);
        memory_free(entry);
        return false;
    }

    return ae;
}


Au_expressions* new_Au_expressions(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Au_expressions* ae = memory_alloc_item(Au_expressions);
    if (ae == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit expressions");
        return NULL;
    }

    ae->entries = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Entry);
    if (ae->entries == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit expressions");
        del_Au_expressions(ae);
        return NULL;
    }

    if (!Streader_read_dict(sr, read_expressions, ae))
    {
        assert(Streader_is_error_set(sr));
        del_Au_expressions(ae);
        return NULL;
    }

    return ae;
}


const Param_proc_filter* Au_expressions_get_proc_filter(
        const Au_expressions* ae, const char* name)
{
    assert(ae != NULL);
    assert(name != NULL);

    if (strlen(name) >= KQT_VAR_NAME_MAX)
        return NULL;

    Entry* key = ENTRY_AUTO;
    strcpy(key->name, name);

    const Entry* entry = AAtree_get_exact(ae->entries, key);
    if (entry == NULL)
        return NULL;

    return entry->filter;
}


void del_Au_expressions(Au_expressions* ae)
{
    if (ae == NULL)
        return;

    del_AAtree(ae->entries);
    memory_free(ae);

    return;
}


