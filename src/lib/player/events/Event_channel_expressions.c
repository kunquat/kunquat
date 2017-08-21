

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
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
#include <player/Channel.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool Event_channel_set_ch_expression_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_NONE ||
            params->arg->type == VALUE_TYPE_STRING);

    set_active_name(&channel->parent, ACTIVE_CAT_CH_EXPRESSION, params->arg);
    const char* expr = channel->init_ch_expression;
    if (params->arg->type == VALUE_TYPE_STRING)
        expr = params->arg->value.string_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(channel, i);

        Voice_state* vstate = channel->fg[i]->state;
        if (!vstate->expr_filters_applied)
            strcpy(vstate->ch_expr_name, expr);
    }

    return true;
}


bool Event_channel_set_note_expression_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_NONE ||
            params->arg->type == VALUE_TYPE_STRING);

    static const char apply_default[] = "";
    static const char disabled[] = "!none";
    const char* expr = apply_default;
    if (params->arg->type == VALUE_TYPE_STRING)
    {
        if (params->arg->value.string_type[0] == '\0')
            expr = disabled;
        else
            expr = params->arg->value.string_type;
    }

    Active_names_set(channel->parent.active_names, ACTIVE_CAT_NOTE_EXPRESSION, expr);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(channel, i);

        Voice_state* vstate = channel->fg[i]->state;
        if (!vstate->expr_filters_applied)
            strcpy(vstate->note_expr_name, expr);
    }

    return true;
}


bool Event_channel_carry_note_expression_on_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    channel->carry_note_expression = true;

    return true;
}


bool Event_channel_carry_note_expression_off_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    channel->carry_note_expression = false;

    return true;
}


