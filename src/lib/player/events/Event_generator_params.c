

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <devices/Generator.h>
#include <player/Active_names.h>
#include <player/events/Event_common.h>
#include <player/events/Event_generator_decl.h>
#include <player/events/set_active_name.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>


bool Event_generator_set_bool_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_BOOL);
    if (string_eq(key, ""))
        return true;

    Device_impl_update_state_bool(dimpl, dstate, key, value->value.bool_type);

    return true;
}


bool Event_generator_set_bool_name_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_BOOL,
            value);
}


bool Event_generator_set_float_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_FLOAT);
    if (string_eq(key, ""))
        return true;

    Device_impl_update_state_float(dimpl, dstate, key, value->value.float_type);

    return true;
}


bool Event_generator_set_float_name_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_FLOAT,
            value);
}


bool Event_generator_set_int_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_INT);
    if (string_eq(key, ""))
        return true;

    Device_impl_update_state_int(dimpl, dstate, key, value->value.int_type);

    return true;
}


bool Event_generator_set_int_name_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_INT,
            value);
}


bool Event_generator_set_tstamp_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_TSTAMP);
    if (string_eq(key, ""))
        return true;

    Device_impl_update_state_tstamp(
            dimpl, dstate, key, &value->value.Tstamp_type);

    return true;
}


bool Event_generator_set_tstamp_name_process(
        const Device_impl* dimpl,
        Device_state* dstate,
        Channel* ch,
        Value* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(ch != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_GEN,
            ACTIVE_TYPE_TSTAMP,
            value);
}


