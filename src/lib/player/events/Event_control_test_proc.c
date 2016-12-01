

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


#include <player/events/Event_control_decl.h>

#include <debug/assert.h>
#include <player/Channel.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


bool Event_control_set_test_processor_process(
        General_state* global_state, Channel* channel, const Value* value)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_INT);

    rassert(value->value.int_type >= 0);
    rassert(value->value.int_type < KQT_PROCESSORS_MAX);

    channel->use_test_output = true;
    channel->test_proc_index = (int)value->value.int_type;

    return true;
}


bool Event_control_set_test_processor_param_process(
        General_state* global_state, Channel* channel, const Value* value)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    strncpy(channel->test_proc_param, value->value.string_type, KQT_VAR_NAME_MAX - 1);
    channel->test_proc_param[KQT_VAR_NAME_MAX - 1] = '\0';

    return true;
}


