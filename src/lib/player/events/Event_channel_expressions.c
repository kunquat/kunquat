

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
#include <init/devices/Au_expressions.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Param_proc_filter.h>
#include <init/Module.h>
#include <player/events/Event_common.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


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

    // Find our expression processor filter
    const Audio_unit* au =
        Module_get_au_from_input(channel->parent.module, channel->au_input);
    if (au == NULL)
        return true;

    const Au_expressions* ae = Audio_unit_get_expressions(au);
    if (ae == NULL)
        return true;

    const char* expr_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_EXPRESSION);
    const Param_proc_filter* filter = Au_expressions_get_proc_filter(ae, expr_name);
    if (filter == NULL)
        return true;

    // Filter out Voices of excluded processors
    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(channel, i);

        if (!Param_proc_filter_is_proc_allowed(filter, i))
        {
            Voice* voice = channel->fg[i];
            voice->state->active = false;
        }
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


