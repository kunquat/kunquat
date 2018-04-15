

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_control_decl.h>

#include <debug/assert.h>
#include <player/Channel.h>
#include <player/events/Event_params.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


bool Event_control_set_test_processor_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_INT);

    rassert(params->arg->value.int_type >= 0);
    rassert(params->arg->value.int_type < KQT_PROCESSORS_MAX);

    channel->use_test_output = true;
    channel->test_proc_index = (int)params->arg->value.int_type;

    return true;
}


bool Event_control_set_test_processor_param_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_STRING);

    strncpy(channel->test_proc_param,
            params->arg->value.string_type,
            KQT_VAR_NAME_MAX);
    channel->test_proc_param[KQT_VAR_NAME_MAX] = '\0';

    return true;
}


