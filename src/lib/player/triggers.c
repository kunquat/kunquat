

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <expr.h>
#include <player/triggers.h>
#include <xassert.h>


char* get_event_type_info(
        char* desc,
        const Event_names* names,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type)
{
    assert(desc != NULL);
    assert(names != NULL);
    assert(rs != NULL);
    assert(ret_name != NULL);
    assert(ret_type != NULL);

    if (rs->error)
        return desc;

    // Read event name
    desc = read_const_char(desc, '[', rs);
    desc = read_string(desc, ret_name, EVENT_NAME_MAX, rs);
    desc = read_const_char(desc, ',', rs);
    if (rs->error)
        return desc;

    // Check event type
    *ret_type = Event_names_get(names, ret_name);
    if (*ret_type == Event_NONE)
    {
        Read_state_set_error(
                rs,
                "Unsupported event type: %s",
                ret_name);
        return desc;
    }

    assert(Event_is_valid(*ret_type));
    return desc;
}


char* process_expr(
        char* arg_expr,
        Value_type field_type,
        Environment* env,
        Random* random,
        const Value* meta,
        Read_state* rs,
        Value* ret_value)
{
    assert(arg_expr != NULL);
    assert(env != NULL);
    assert(random != NULL);
    assert(rs != NULL);
    assert(ret_value != NULL);

    if (rs->error)
        return arg_expr;

    if (field_type == VALUE_TYPE_NONE)
    {
        ret_value->type = VALUE_TYPE_NONE;
        arg_expr = read_null(arg_expr, rs);
    }
    else
    {
        arg_expr = evaluate_expr(
                arg_expr,
                env,
                rs,
                meta,
                ret_value,
                random);
        arg_expr = read_const_char(arg_expr, '"', rs);

        if (rs->error)
            return arg_expr;

        if (!Value_convert(ret_value, ret_value, field_type))
            Read_state_set_error(rs, "Type mismatch");
    }

    return arg_expr;
}


bool process_trigger(
        char* trigger_desc,
        const Event_names* names,
        Environment* env,
        Random* random,
        const Value* meta,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type,
        Value* ret_value)
{
    assert(trigger_desc != NULL);
    assert(names != NULL);
    assert(env != NULL);
    assert(random != NULL);
    assert(rs != NULL);
    assert(ret_name != NULL);
    assert(ret_type != NULL);
    assert(ret_value != NULL);

    if (rs->error)
        return false;

    trigger_desc = get_event_type_info(
            trigger_desc,
            names,
            rs,
            ret_name,
            ret_type);
    if (rs->error)
        return false;

    // TODO: Handle quoted events (string literal argument!)

    trigger_desc = process_expr(
                trigger_desc,
                Event_names_get_param_type(names, ret_name),
                env,
                random,
                meta,
                rs,
                ret_value);
    if (rs->error)
        return false;

    trigger_desc = read_const_char(trigger_desc, ']', rs);
    return !rs->error;
}


