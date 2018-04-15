

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
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
#include <string/common.h>
#include <string/Streader.h>
#include <string/var_name.h>

#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char name[KQT_VAR_NAME_MAX + 1];
    Param_proc_filter* filter;
} Entry;


#define ENTRY_AUTO (&(Entry){ .name = "", .filter = NULL })


static void del_Entry(Entry* entry)
{
    rassert(entry != NULL);

    del_Param_proc_filter(entry->filter);
    memory_free(entry);

    return;
}


struct Au_expressions
{
    AAtree* entries;
    char default_note_expr[KQT_VAR_NAME_MAX + 1];
};


static bool read_expressions(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

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

    return true;
}


static bool read_expressions_def(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

    Au_expressions* ae = userdata;

    if (string_eq(key, "default_note_expr"))
    {
        char expr[KQT_VAR_NAME_MAX + 2] = "";
        if (!Streader_read_string(sr, KQT_VAR_NAME_MAX + 2, expr))
            return false;

        if ((expr[0] != '\0') && !is_valid_var_name(expr))
        {
            Streader_set_error(sr, "Invalid default note expression: %s", expr);
            return false;
        }

        strcpy(ae->default_note_expr, expr);
    }
    else if (string_eq(key, "expressions"))
    {
        if (!Streader_read_dict(sr, read_expressions, ae))
        {
            rassert(Streader_is_error_set(sr));
            return false;
        }
    }
    else
    {
        Streader_set_error(sr, "Unexpected key in expression specification: %s", key);
        return false;
    }

    return true;
}


Au_expressions* new_Au_expressions(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Au_expressions* ae = memory_alloc_item(Au_expressions);
    if (ae == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit expressions");
        return NULL;
    }

    memset(ae->default_note_expr, '\0', KQT_VAR_NAME_MAX + 1);
    ae->entries = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Entry);
    if (ae->entries == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit expressions");
        del_Au_expressions(ae);
        return NULL;
    }

    if (!Streader_read_dict(sr, read_expressions_def, ae))
    {
        rassert(Streader_is_error_set(sr));
        del_Au_expressions(ae);
        return NULL;
    }

    if (ae->default_note_expr[0] != '\0' &&
            !Au_expressions_get_proc_filter(ae, ae->default_note_expr))
    {
        Streader_set_error(
                sr,
                "Audio unit expressions do not contain the default expression %s",
                ae->default_note_expr);
        del_Au_expressions(ae);
        return NULL;
    }

    return ae;
}


const char* Au_expressions_get_default_note_expr(const Au_expressions* ae)
{
    rassert(ae != NULL);
    return ae->default_note_expr;
}


const Param_proc_filter* Au_expressions_get_proc_filter(
        const Au_expressions* ae, const char* name)
{
    rassert(ae != NULL);
    rassert(name != NULL);

    if (strlen(name) > KQT_VAR_NAME_MAX)
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


