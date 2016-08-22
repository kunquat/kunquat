

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


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


bool Event_channel_set_init_expression_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    return set_active_name(&channel->parent, ACTIVE_CAT_INIT_EXPRESSION, value);
}


bool Event_channel_apply_expression_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    set_active_name(&channel->parent, ACTIVE_CAT_EXPRESSION, value);
    const char* expr = value->value.string_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(channel, i);

        Voice* voice = channel->fg[i];
        strcpy(voice->state->expr_name, expr);
    }

    return true;
}


bool Event_channel_carry_expression_on_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    channel->carry_expression = true;

    return true;
}


bool Event_channel_carry_expression_off_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    channel->carry_expression = false;

    return true;
}


