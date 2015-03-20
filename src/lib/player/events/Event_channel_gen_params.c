

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
#include <stdbool.h>

#include <debug/assert.h>
#include <player/Active_names.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <player/events/set_active_name.h>
#include <Value.h>


bool Event_channel_set_gen_bool_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    const char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_BOOL);

    return Channel_proc_state_modify_value(ch->cpstate, key, value);
}


bool Event_channel_set_gen_bool_name_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_BOOL,
            value);
}


bool Event_channel_set_gen_float_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_FLOAT);

    return Channel_proc_state_modify_value(ch->cpstate, key, value);
}


bool Event_channel_set_gen_float_name_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_FLOAT,
            value);
}


bool Event_channel_set_gen_int_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    const char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_INT);

    return Channel_proc_state_modify_value(ch->cpstate, key, value);
}


bool Event_channel_set_gen_int_name_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_INT,
            value);
}


bool Event_channel_set_gen_tstamp_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* key = Active_names_get(
            ch->parent.active_names,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_TSTAMP);

    return Channel_proc_state_modify_value(ch->cpstate, key, value);
}


bool Event_channel_set_gen_tstamp_name_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_TSTAMP,
            value);
}


