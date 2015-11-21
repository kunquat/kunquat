

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


#include <stdbool.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/set_active_name.h>
#include <Value.h>


bool Event_channel_set_cv_bool_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_BOOL, value);
}


bool Event_channel_set_cv_int_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_INT, value);
}


bool Event_channel_set_cv_float_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT, value);
}


bool Event_channel_set_cv_tstamp_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_TSTAMP, value);
}


