

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_control_vars.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <expr.h>
#include <mathnum/common.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <string/common.h>
#include <string/Streader.h>
#include <string/var_name.h>
#include <Value.h>

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Au_control_vars
{
    AAtree* vars;
};


typedef enum
{
    ITER_MODE_DEVICES,
    ITER_MODE_SET_GENERIC,
} Iter_mode;


typedef enum
{
    BIND_ENTRY_TYPE_INVALID,
    BIND_ENTRY_TYPE_EXPRESSION,
} Bind_entry_type;


typedef struct Bind_entry
{
    Bind_entry_type type;

    Target_dev_type target_dev_type;
    int target_dev_index;

    Value_type target_var_type;
    char target_var_name[KQT_VAR_NAME_MAX + 1];

    // Type-specific settings
    union
    {
        struct
        {
            char* expression;
        } expr_type;
    } ext;

    struct Bind_entry* next;
} Bind_entry;


static void del_Bind_entry(Bind_entry* entry)
{
    if (entry == NULL)
        return;

    if (entry->type == BIND_ENTRY_TYPE_EXPRESSION)
        memory_free(entry->ext.expr_type.expression);

    memory_free(entry);

    return;
}


typedef enum
{
    VAR_ENTRY_INVALID,
    VAR_ENTRY_BOOL,
    VAR_ENTRY_INT,
    VAR_ENTRY_FLOAT,
    VAR_ENTRY_TSTAMP,
} Var_entry_type;


typedef struct Var_entry
{
    char name[KQT_VAR_NAME_MAX + 1];
    Var_entry_type type;
    Value init_value;

    Bind_entry* first_bind_entry;
    Bind_entry* last_bind_entry;
} Var_entry;


static void del_Var_entry(Var_entry* entry)
{
    if (entry == NULL)
        return;

    Bind_entry* cur = entry->first_bind_entry;
    while (cur != NULL)
    {
        Bind_entry* next = cur->next;
        del_Bind_entry(cur);
        cur = next;
    }

    memory_free(entry);

    return;
}


static Value_type get_value_type_from_var_entry_type(Var_entry_type var_entry_type)
{
    switch (var_entry_type)
    {
        case VAR_ENTRY_BOOL: return VALUE_TYPE_BOOL;
        case VAR_ENTRY_INT:  return VALUE_TYPE_INT;
        case VAR_ENTRY_FLOAT: return VALUE_TYPE_FLOAT;
        case VAR_ENTRY_TSTAMP: return VALUE_TYPE_TSTAMP;

        default:
            rassert(false);
    }

    return VALUE_TYPE_NONE;
}


Au_control_var_iter* Au_control_var_iter_init(
        Au_control_var_iter* iter, const Au_control_vars* aucv)
{
    rassert(iter != NULL);
    rassert(aucv != NULL);

    // Get the first entry
    AAiter_init(&iter->iter, aucv->vars);
    const Var_entry* var_entry = AAiter_get_at_least(&iter->iter, "");
    if (var_entry != NULL)
    {
        iter->next_var_name = var_entry->name;
        iter->next_var_type = get_value_type_from_var_entry_type(var_entry->type);
    }
    else
    {
        iter->next_var_name = NULL;
        iter->next_var_type = VALUE_TYPE_NONE;
    }

    return iter;
}


void Au_control_var_iter_get_next_var_info(
        Au_control_var_iter* iter, const char** out_var_name, Value_type* out_var_type)
{
    rassert(iter != NULL);
    rassert(out_var_name != NULL);
    rassert(out_var_type != NULL);

    *out_var_name = iter->next_var_name;
    *out_var_type = iter->next_var_type;

    // Prepare for the next call
    if (iter->next_var_name != NULL)
    {
        const Var_entry* next_entry = AAiter_get_next(&iter->iter);
        if (next_entry != NULL)
        {
            iter->next_var_name = next_entry->name;
            iter->next_var_type = get_value_type_from_var_entry_type(next_entry->type);
        }
        else
        {
            iter->next_var_name = NULL;
            iter->next_var_type = VALUE_TYPE_NONE;
        }
    }

    return;
}


static bool Au_control_binding_iter_init_common(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name,
        Iter_mode iter_mode)
{
    rassert(iter != NULL);
    rassert(aucv != NULL);
    rassert(var_name != NULL);

    const Var_entry* var_entry = AAtree_get_exact(aucv->vars, var_name);
    if (var_entry == NULL)
        return false;

    iter->iter = var_entry->first_bind_entry;
    if (iter->iter == NULL)
        return false;

    iter->iter_mode = iter_mode;
    iter->rand = NULL;

    return true;
}


static void Au_control_binding_iter_update(Au_control_binding_iter* iter)
{
    rassert(iter != NULL);
    rassert(iter->iter != NULL);

    iter->target_dev_type = iter->iter->target_dev_type;
    iter->target_dev_index = iter->iter->target_dev_index;
    iter->target_var_name = iter->iter->target_var_name;

    switch (iter->iter_mode)
    {
        case ITER_MODE_DEVICES:
        {
        }
        break;

        case ITER_MODE_SET_GENERIC:
        {
            rassert(iter->rand != NULL);

            const char* expr = iter->iter->ext.expr_type.expression;
            rassert(expr != NULL);

            Streader* sr = Streader_init(STREADER_AUTO, expr, (int64_t)strlen(expr));
            Value* result = VALUE_AUTO;

            if (evaluate_expr(sr, NULL, &iter->src_value, result, iter->rand))
            {
                if (!Value_convert(result, result, iter->iter->target_var_type))
                    result->type = VALUE_TYPE_NONE;
            }
            else
            {
                result->type = VALUE_TYPE_NONE;
            }

            Value_copy(&iter->target_value, result);
        }
        break;

        default:
            rassert(false);
    }

    return;
}


bool Au_control_binding_iter_init(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name)
{
    rassert(iter != NULL);
    rassert(aucv != NULL);
    rassert(var_name != NULL);

    iter->iter = NULL;
    iter->src_value.type = VALUE_TYPE_NONE;

    if (!Au_control_binding_iter_init_common(iter, aucv, var_name, ITER_MODE_DEVICES))
        return false;

    iter->target_value.type = VALUE_TYPE_NONE;

    Au_control_binding_iter_update(iter);

    return true;
}


bool Au_control_binding_iter_init_set_generic(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        Random* rand,
        const char* var_name,
        const Value* value)
{
    rassert(iter != NULL);
    rassert(aucv != NULL);
    rassert(rand != NULL);
    rassert(var_name != NULL);
    rassert(value != NULL);
    rassert(Value_type_is_realtime(value->type));

    iter->iter = NULL;
    Value_copy(&iter->src_value, value);

    if (!Au_control_binding_iter_init_common(
            iter, aucv, var_name, ITER_MODE_SET_GENERIC))
        return false;

    const Var_entry* var_entry = AAtree_get_exact(aucv->vars, var_name);
    rassert(var_entry != NULL);

    // Try to convert input value to type expected by the variable entry
    if (!Value_convert(
                &iter->src_value,
                &iter->src_value,
                get_value_type_from_var_entry_type(var_entry->type)))
        return false;

    iter->rand = rand;

    Au_control_binding_iter_update(iter);

    return true;
}


bool Au_control_binding_iter_get_next_entry(Au_control_binding_iter* iter)
{
    rassert(iter != NULL);
    rassert(iter->iter != NULL);

    iter->iter = iter->iter->next;

    if (iter->iter == NULL)
        return false;

    Au_control_binding_iter_update(iter);

    return true;
}


static const char* mem_error_str =
    "Could not allocate memory for audio unit control variables";


static Bind_entry* new_Bind_entry_common(Streader* sr)
{
    rassert(sr != NULL);

    char target_dev_name[16] = "";
    char target_var_name[KQT_VAR_NAME_MAX + 2] = "";

    if (!Streader_readf(
                sr,
                "%s,%s,",
                READF_STR(16, target_dev_name),
                READF_STR(KQT_VAR_NAME_MAX + 2, target_var_name)))
        return NULL;

    // Parse target device name
    Target_dev_type target_dev_type = TARGET_DEV_NONE;
    int target_dev_index = -1;
    if (string_has_prefix(target_dev_name, "au_"))
    {
        target_dev_type = TARGET_DEV_AU;
        target_dev_index = string_extract_index(target_dev_name, "au_", 2, "");
    }
    else if (string_has_prefix(target_dev_name, "proc_"))
    {
        target_dev_type = TARGET_DEV_PROC;
        target_dev_index = string_extract_index(target_dev_name, "proc_", 2, "");
    }

    if ((target_dev_type == TARGET_DEV_NONE) || (target_dev_index < 0))
    {
        Streader_set_error(sr, "Invalid target device name: %s", target_dev_name);
        return NULL;
    }

    // Check target variable path
    if (!is_valid_var_path(target_var_name))
    {
        Streader_set_error(
                sr,
                "Illegal target variable path %s"
                    " (Variable path components must contain"
                    " only lower-case letters and underscores"
                    " (and digits as other than first characters))",
                target_var_name);
        return NULL;
    }

    Bind_entry* bind_entry = memory_alloc_item(Bind_entry);
    if (bind_entry == NULL)
    {
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    // Initialise common fields
    bind_entry->type = BIND_ENTRY_TYPE_INVALID;
    bind_entry->target_dev_type = target_dev_type;
    bind_entry->target_dev_index = target_dev_index;
    bind_entry->target_var_type = VALUE_TYPE_NONE;
    strcpy(bind_entry->target_var_name, target_var_name);
    bind_entry->next = NULL;

    return bind_entry;
}


static bool read_binding_targets_generic(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);

    Var_entry* entry = userdata;
    rassert(entry != NULL);

    if (!Streader_readf(sr, "["))
        return false;

    // Create and attach new bind entry
    Bind_entry* bind_entry = new_Bind_entry_common(sr);
    if (bind_entry == NULL)
        return false;

    if (entry->last_bind_entry == NULL)
    {
        rassert(entry->first_bind_entry == NULL);
        entry->first_bind_entry = bind_entry;
        entry->last_bind_entry = bind_entry;
    }
    else
    {
        entry->last_bind_entry->next = bind_entry;
        entry->last_bind_entry = bind_entry;
    }

    // Get target variable type
    char type_name[16] = "";
    if (!Streader_readf(sr, "%s,", READF_STR(16, type_name)))
        return false;

    if (string_eq(type_name, "bool"))
        bind_entry->target_var_type = VALUE_TYPE_BOOL;
    else if (string_eq(type_name, "int"))
        bind_entry->target_var_type = VALUE_TYPE_INT;
    else if (string_eq(type_name, "float"))
        bind_entry->target_var_type = VALUE_TYPE_FLOAT;
    else if (string_eq(type_name, "tstamp"))
        bind_entry->target_var_type = VALUE_TYPE_TSTAMP;

    if (bind_entry->target_var_type == VALUE_TYPE_NONE)
    {
        Streader_set_error(sr, "Invalid target variable type: %s", type_name);
        return false;
    }

    bind_entry->type = BIND_ENTRY_TYPE_EXPRESSION;
    bind_entry->ext.expr_type.expression = NULL;

    // Get memory area of the expression string
    Streader_skip_whitespace(sr);
    const char* const expr = Streader_get_remaining_data(sr);
    if (!Streader_read_string(sr, 0, NULL))
        return false;

    const char* const expr_end = Streader_get_remaining_data(sr);

    if (!Streader_readf(sr, "]"))
        return false;

    // Allocate space for the expression string
    rassert(expr_end != NULL);
    rassert(expr_end > expr);
    const ptrdiff_t expr_length = expr_end - expr;

    bind_entry->ext.expr_type.expression = memory_calloc_items(char, expr_length + 1);
    if (bind_entry->ext.expr_type.expression == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit control variables");
        return false;
    }

    strncpy(bind_entry->ext.expr_type.expression, expr, (size_t)expr_length);
    bind_entry->ext.expr_type.expression[expr_length] = '\0';

    return true;
}


static bool read_var_entry(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);

    Au_control_vars* acv = userdata;
    rassert(acv != NULL);

    // Read type and name
    char type_name[16] = "";
    char var_name[KQT_VAR_NAME_MAX + 2] = "";

    if (!Streader_readf(
                sr,
                "[%s,%s,",
                READF_STR(16, type_name),
                READF_STR(KQT_VAR_NAME_MAX + 2, var_name)))
        return false;

    if (!is_valid_var_name(var_name))
    {
        Streader_set_error(
                sr,
                "Illegal variable name %s"
                    " (Variable names may only contain"
                    " lower-case letters and underscores"
                    " (and digits as other than first characters))",
                var_name);
        return false;
    }

    // Create and add new variable entry
    Var_entry* entry = memory_alloc_item(Var_entry);
    if (entry == NULL)
    {
        Streader_set_memory_error(sr, mem_error_str);
        return false;
    }

    strcpy(entry->name, var_name);
    entry->type = VAR_ENTRY_INVALID;
    entry->init_value = *VALUE_AUTO;
    entry->first_bind_entry = NULL;
    entry->last_bind_entry = NULL;

    if (!AAtree_ins(acv->vars, entry))
    {
        memory_free(entry);
        Streader_set_memory_error(sr, mem_error_str);
        return false;
    }

    // Read type-specific parts
    if (string_eq(type_name, "bool"))
    {
        entry->type = VAR_ENTRY_BOOL;

        entry->init_value.type = VALUE_TYPE_BOOL;

        if (!Streader_readf(
                    sr,
                    "%b,[],%l]",
                    &entry->init_value.value.bool_type,
                    read_binding_targets_generic,
                    entry))
            return false;
    }
    else if (string_eq(type_name, "int"))
    {
        entry->type = VAR_ENTRY_INT;

        entry->init_value.type = VALUE_TYPE_INT;

        if (!Streader_readf(
                    sr,
                    "%i,[],%l]",
                    &entry->init_value.value.int_type,
                    read_binding_targets_generic,
                    entry))
            return false;
    }
    else if (string_eq(type_name, "float"))
    {
        entry->type = VAR_ENTRY_FLOAT;

        entry->init_value.type = VALUE_TYPE_FLOAT;

        if (!Streader_readf(
                    sr,
                    "%f,[],%l]",
                    &entry->init_value.value.float_type,
                    read_binding_targets_generic,
                    entry))
            return false;
    }
    else if (string_eq(type_name, "tstamp"))
    {
        entry->type = VAR_ENTRY_TSTAMP;

        entry->init_value.type = VALUE_TYPE_TSTAMP;

        if (!Streader_readf(
                    sr,
                    "%t,[],%l]",
                    &entry->init_value.value.Tstamp_type,
                    read_binding_targets_generic,
                    entry))
            return false;
    }
    else
    {
        Streader_set_error(
                sr,
                "Invalid type of audio unit control variable %s: %s",
                var_name,
                type_name);
        return false;
    }

    return true;
}


Au_control_vars* new_Au_control_vars(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Au_control_vars* aucv = memory_alloc_item(Au_control_vars);
    if (aucv == NULL)
    {
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    aucv->vars = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Var_entry);
    if (aucv->vars == NULL)
    {
        del_Au_control_vars(aucv);
        Streader_set_memory_error(sr, mem_error_str);
        return NULL;
    }

    if (!Streader_read_list(sr, read_var_entry, aucv))
    {
        del_Au_control_vars(aucv);
        return NULL;
    }

    return aucv;
}


const Value* Au_control_vars_get_init_value(
        const Au_control_vars* aucv, const char* var_name)
{
    rassert(aucv != NULL);
    rassert(var_name != NULL);

    const Var_entry* entry = AAtree_get_exact(aucv->vars, var_name);
    rassert(entry != NULL);

    return &entry->init_value;
}


void del_Au_control_vars(Au_control_vars* aucv)
{
    if (aucv == NULL)
        return;

    del_AAtree(aucv->vars);
    memory_free(aucv);

    return;
}


