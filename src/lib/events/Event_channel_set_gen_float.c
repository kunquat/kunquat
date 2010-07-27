

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <Event_common.h>
#include <Event_channel_set_gen_float.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_gen_float_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_GEN_FLOAT,
                         set_gen_float);


bool Event_channel_set_gen_float_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Read_state* state = READ_STATE_AUTO;
    fields = read_const_char(fields, '[', state);
    char key[100] = { '\0' };
    fields = read_string(fields, key, 99, state);
    fields = read_const_char(fields, ',', state);
    if (state->error || !string_has_suffix(key, ".jsonf"))
    {
        return false;
    }
    return Channel_gen_state_modify_value(ch_state->cgstate, key, fields);
}


