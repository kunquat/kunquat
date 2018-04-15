

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_event_map.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <decl.h>
#include <expr.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <string/common.h>
#include <string/device_event_name.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct Au_event_map
{
    AAtree* tree;
};


struct Au_event_bind_entry
{
    Au_event_target_dev_type target_dev_type;
    int target_dev_index;

    char target_event_name[KQT_DEVICE_EVENT_NAME_MAX + 1];

    Value_type target_arg_type;
    char* expression;

    struct Au_event_bind_entry* next;
};


static void del_Bind_entry(Au_event_bind_entry* entry)
{
    if (entry == NULL)
        return;

    memory_free(entry->expression);
    memory_free(entry);

    return;
}


static bool get_argument_type(const char* type_name, Value_type* out_type)
{
    rassert(type_name != NULL);
    rassert(out_type != NULL);

    static const struct
    {
        const char* name;
        Value_type type;
    } type_name_map[] =
    {
        { "none",   VALUE_TYPE_NONE },
        { "bool",   VALUE_TYPE_BOOL },
        { "int",    VALUE_TYPE_INT },
        { "float",  VALUE_TYPE_FLOAT },
        { "tstamp", VALUE_TYPE_TSTAMP },
        { NULL,     VALUE_TYPE_NONE },
    };

    int i = 0;
    while (type_name_map[i].name != NULL)
    {
        if (string_eq(type_name, type_name_map[i].name))
        {
            *out_type = type_name_map[i].type;
            return true;
        }
        ++i;
    }

    return false;
}


static const char* mem_error_str =
    "Could not allocate memory for audio unit event map";


static Au_event_bind_entry* new_Bind_entry(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    char target_dev_name[16] = "";
    char target_event_name[KQT_DEVICE_EVENT_NAME_MAX + 2] = "";
    char target_event_arg_type_name[16] = "";

    if (!Streader_readf(
                sr,
                "[%s,%s,%s,",
                READF_STR(16, target_dev_name),
                READF_STR(KQT_DEVICE_EVENT_NAME_MAX + 2, target_event_name),
                READF_STR(16, target_event_arg_type_name)))
        return NULL;

    // Get target device information
    Au_event_target_dev_type target_dev_type = AU_EVENT_TARGET_DEV_AU;
    int target_dev_index = -1;
    {
        static const struct
        {
            const char* prefix;
            Au_event_target_dev_type type;
        } target_dev_type_map[] =
        {
            { "au_",    AU_EVENT_TARGET_DEV_AU },
            { "proc_",  AU_EVENT_TARGET_DEV_PROC },
            { NULL,     AU_EVENT_TARGET_DEV_AU }
        };

        int i = 0;
        while (target_dev_type_map[i].prefix != NULL)
        {
            if (string_has_prefix(target_dev_name, target_dev_type_map[i].prefix) &&
                    strlen(target_dev_name) == strlen(target_dev_type_map[i].prefix) + 2)
            {
                target_dev_type = target_dev_type_map[i].type;
                target_dev_index = string_extract_index(
                        target_dev_name, target_dev_type_map[i].prefix, 2, "");
                break;
            }
            ++i;
        }
    }

    if (target_dev_index < 0)
    {
        Streader_set_error(sr, "Invalid target device name: %s", target_dev_name);
        return NULL;
    }

    // Validate target event name
    if (!is_valid_device_event_name(target_event_name))
    {
        Streader_set_error(
                sr,
                "Invalid device event name %s"
                " (device event names must be less than %s characters"
                " and may only contain lower-case letters)",
                target_event_name,
                KQT_DEVICE_EVENT_NAME_MAX);
        return NULL;
    }

    // Validate target event argument type
    Value_type target_arg_type = VALUE_TYPE_NONE;
    if (!get_argument_type(target_event_arg_type_name, &target_arg_type))
    {
        Streader_set_error(
                sr,
                "Invalid device event argument type %s"
                " (must be one of: none, bool, int, float, tstamp)",
                target_event_arg_type_name);
        return false;
    }

    // Get target event argument expression
    char* expression = NULL;

    /*
    if (target_arg_type == VALUE_TYPE_NONE)
    {
        if (!Streader_read_null(sr))
        {
            Streader_set_error(
                    sr,
                    "target event argument type of none should be followed by a null");
            return false;
        }

        if (!Streader_readf(sr, "]"))
            return false;
    }
    else
    // */
    {
        Streader_skip_whitespace(sr);
        const char* const expr = Streader_get_remaining_data(sr);
        if (!Streader_read_string(sr, 0, NULL))
            return false;

        const char* const expr_end = Streader_get_remaining_data(sr);

        if (!Streader_readf(sr, "]"))
            return false;

        rassert(expr_end != NULL);
        rassert(expr_end > expr);
        const ptrdiff_t expr_length = expr_end - expr;

        expression = memory_calloc_items(char, expr_length + 1);
        if (expression == NULL)
        {
            Streader_set_memory_error(sr, mem_error_str);
            return false;
        }

        strncpy(expression, expr, (size_t)expr_length);
        expression[expr_length] = '\0';
    }

    Au_event_bind_entry* bind_entry = memory_alloc_item(Au_event_bind_entry);
    if (bind_entry == NULL)
    {
        memory_free(expression);
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    bind_entry->target_dev_type = target_dev_type;
    bind_entry->target_dev_index = target_dev_index;
    strcpy(bind_entry->target_event_name, target_event_name);
    bind_entry->target_arg_type = target_arg_type;
    bind_entry->expression = expression;
    bind_entry->next = NULL;

    return bind_entry;
}


typedef struct Event_entry
{
    char event_name[KQT_DEVICE_EVENT_NAME_MAX + 1];
    Value_type event_arg_type;

    Au_event_bind_entry* first_bind_entry;
    Au_event_bind_entry* last_bind_entry;
} Event_entry;


static void del_Event_entry(Event_entry* entry)
{
    if (entry == NULL)
        return;

    Au_event_bind_entry* cur = entry->first_bind_entry;
    while (cur != NULL)
    {
        Au_event_bind_entry* next = cur->next;
        del_Bind_entry(cur);
        cur = next;
    }

    memory_free(entry);

    return;
}


static bool read_bind_targets(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);

    Event_entry* event_entry = userdata;
    rassert(event_entry != NULL);

    // Read and attach new bind entry
    Au_event_bind_entry* bind_entry = new_Bind_entry(sr);
    if (bind_entry == NULL)
        return false;

    if (event_entry->last_bind_entry == NULL)
    {
        rassert(event_entry->first_bind_entry == NULL);
        event_entry->first_bind_entry = bind_entry;
        event_entry->last_bind_entry = bind_entry;
    }
    else
    {
        event_entry->last_bind_entry->next = bind_entry;
        event_entry->last_bind_entry = bind_entry;
    }

    return true;
}


static bool read_event_entry(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);

    Au_event_map* map = userdata;
    rassert(map != NULL);

    // Read event name and argument type
    char event_name[KQT_DEVICE_EVENT_NAME_MAX + 2] = "";
    char arg_type_name[16] = "";
    Value_type arg_type = VALUE_TYPE_NONE;

    if (!Streader_readf(
                sr,
                "[%s,%s,",
                READF_STR(KQT_DEVICE_EVENT_NAME_MAX + 2, event_name),
                READF_STR(16, arg_type_name)))
        return false;

    if (!is_valid_device_event_name(event_name))
    {
        Streader_set_error(
                sr,
                "Invalid device event name %s"
                " (device event names must not be more than %d characters"
                " and may only contain lower-case letters)",
                event_name,
                KQT_DEVICE_EVENT_NAME_MAX);
        return false;
    }

    if (!get_argument_type(arg_type_name, &arg_type))
    {
        Streader_set_error(
                sr,
                "Invalid device event argument type %s"
                " (must be one of: none, bool, int, float, tstamp)",
                arg_type_name);
        return false;
    }

    // Create and add new event entry
    Event_entry* entry = memory_alloc_item(Event_entry);
    if (entry == NULL)
    {
        Streader_set_memory_error(sr, mem_error_str);
        return false;
    }

    strcpy(entry->event_name, event_name);
    entry->event_arg_type = arg_type;
    entry->first_bind_entry = NULL;
    entry->last_bind_entry = NULL;

    if (!AAtree_ins(map->tree, entry))
    {
        memory_free(entry);
        Streader_set_memory_error(sr, mem_error_str);
        return false;
    }

    return Streader_readf(sr, "%l]", read_bind_targets, entry);
}


static void fill_iter_result(Au_event_iter* iter)
{
    rassert(iter != NULL);
    rassert(iter->bind_entry != NULL);

    const Au_event_bind_entry* entry = iter->bind_entry;

    iter->result.dev_type = entry->target_dev_type;
    iter->result.dev_index = entry->target_dev_index;
    iter->result.event_name = entry->target_event_name;

    if (entry->target_arg_type != VALUE_TYPE_NONE)
    {
        Streader* sr = Streader_init(
                STREADER_AUTO, entry->expression, (int64_t)strlen(entry->expression));
        Value* meta = (iter->src_value.type != VALUE_TYPE_NONE) ? &iter->src_value : NULL;
        Value* result = &iter->result.arg;
        if (evaluate_expr(sr, NULL, meta, result, iter->rand))
        {
            if (!Value_convert(result, result, entry->target_arg_type))
                result->type = VALUE_TYPE_NONE;
        }
        else
        {
            result->type = VALUE_TYPE_NONE;
        }
    }
    else
    {
        iter->result.arg.type = VALUE_TYPE_NONE;
    }

    return;
}


Au_event_iter_result* Au_event_iter_init(
        Au_event_iter* iter,
        const Au_event_map* map,
        const char* event_name,
        const Value* arg,
        Random* rand)
{
    rassert(iter != NULL);
    rassert(map != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);
    rassert(rand != NULL);

    const Event_entry* entry = AAtree_get_exact(map->tree, event_name);
    if (entry == NULL)
    {
        iter->bind_entry = NULL;
        return NULL;
    }

    // Check and convert arg if needed
    if (entry->event_arg_type != VALUE_TYPE_NONE)
    {
        if (entry->event_arg_type != arg->type)
        {
            if (!Value_convert(&iter->src_value, arg, entry->event_arg_type))
                return NULL;
        }
        else
        {
            Value_copy(&iter->src_value, arg);
        }
    }
    else
    {
        iter->src_value.type = VALUE_TYPE_NONE;
    }

    iter->bind_entry = entry->first_bind_entry;
    iter->rand = rand;

    fill_iter_result(iter);

    return &iter->result;
}


Au_event_iter_result* Au_event_iter_get_next(Au_event_iter* iter)
{
    rassert(iter != NULL);

    if (iter->bind_entry == NULL)
        return NULL;

    iter->bind_entry = iter->bind_entry->next;
    if (iter->bind_entry == NULL)
        return NULL;

    fill_iter_result(iter);

    return &iter->result;
}


Au_event_map* new_Au_event_map(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Au_event_map* map = memory_alloc_item(Au_event_map);
    if (map == NULL)
    {
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    map->tree = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Event_entry);
    if (map->tree == NULL)
    {
        del_Au_event_map(map);
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    if (!Streader_read_list(sr, read_event_entry, map))
    {
        del_Au_event_map(map);
        return NULL;
    }

    return map;
}


void del_Au_event_map(Au_event_map* map)
{
    if (map == NULL)
        return;

    del_AAtree(map->tree);
    memory_free(map);

    return;
}


