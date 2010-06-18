

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
#include <assert.h>
#include <stdbool.h>

#include <Event_common.h>
#include <Event_channel_set_gen_bool.h>
#include <string_common.h>

#include <xmemory.h>


static Event_field_desc set_gen_bool_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_BOOL
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_channel_set_gen_bool_set(Event* event, int index, void* data);


static void* Event_channel_set_gen_bool_get(Event* event, int index);


Event_create_constructor(Event_channel_set_gen_bool,
                         EVENT_CHANNEL_SET_GEN_BOOL,
                         set_gen_bool_desc,
                         event->value = false);


static bool Event_channel_set_gen_bool_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SET_GEN_BOOL);
    Event_channel_set_gen_bool* set_gen_bool = (Event_channel_set_gen_bool*)event;
    if (index == 1)
    {
        assert(data != NULL);
        set_gen_bool->value = *(bool*)data;
        return true;
    }
    return false;
}


static void* Event_channel_set_gen_bool_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SET_GEN_BOOL);
    Event_channel_set_gen_bool* set_gen_bool = (Event_channel_set_gen_bool*)event;
    if (index == 1)
    {
        return &set_gen_bool->value;
    }
    return NULL;
}


bool Event_channel_set_gen_bool_process(Channel_state* ch_state, char* fields)
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
    if (state->error || !string_has_suffix(key, ".jsonb"))
    {
        return false;
    }
    return Channel_gen_state_modify_value(ch_state->cgstate, key, fields);
}


