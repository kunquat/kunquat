

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
#include <stdbool.h>

#include <Active_names.h>
#include <Event_common.h>
#include <Event_generator_decl.h>
#include <File_base.h>
#include <Generator.h>
#include <set_active_name.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>


bool Event_generator_set_bool_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    char* key = Active_names_get(
            ch_state->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_BOOL);
    if (!string_has_suffix(key, ".jsonb"))
    {
        return true;
    }
    return Device_params_modify_value(
            gen->conf->params,
            key,
            &value->value.bool_type);
}


bool Event_generator_set_bool_name_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)gen;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(
            &ch_state->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_BOOL,
            value);
}


bool Event_generator_set_float_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    char* key = Active_names_get(
            ch_state->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_FLOAT);
    if (!string_has_suffix(key, ".jsonf"))
    {
        return true;
    }
    return Device_params_modify_value(
            gen->conf->params,
            key,
            &value->value.float_type);
}


bool Event_generator_set_float_name_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)gen;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(
            &ch_state->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_FLOAT,
            value);
}


bool Event_generator_set_int_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    char* key = Active_names_get(
            ch_state->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_INT);
    if (!string_has_suffix(key, ".jsoni"))
    {
        return true;
    }
    return Device_params_modify_value(
            gen->conf->params,
            key,
            &value->value.int_type);
}


bool Event_generator_set_int_name_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)gen;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(
            &ch_state->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_INT,
            value);
}


bool Event_generator_set_tstamp_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    char* key = Active_names_get(
            ch_state->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_TSTAMP);
    if (!string_has_suffix(key, ".jsont"))
    {
        return true;
    }
    return Device_params_modify_value(
            gen->conf->params,
            key,
            &value->value.Tstamp_type);
}


bool Event_generator_set_tstamp_name_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)gen;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(
            &ch_state->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_TSTAMP,
            value);
}


