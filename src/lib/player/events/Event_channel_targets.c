

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
#include <kunquat/limits.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <Value.h>


bool Event_channel_set_ins_input_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    ch->ins_input = value->value.int_type;

    return true;
}


bool Event_channel_set_processor_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    ch->processor = value->value.int_type;

    return true;
}


bool Event_channel_set_global_effects_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    (void)value;

    ch->inst_effects = false;

    return true;
}


bool Event_channel_set_instrument_effects_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    (void)value;

    ch->inst_effects = true;

    return true;
}


bool Event_channel_set_effect_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    ch->effect = value->value.int_type;

    return true;
}


