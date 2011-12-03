

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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

#include <Active_names.h>
#include <Event.h>
#include <Event_common.h>
#include <Event_channel_set_gen_int_name.h>
#include <set_active_name.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc* set_gen_int_name_desc = set_name_desc;


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_GEN_INT_NAME,
                         set_gen_int_name);


bool Event_channel_set_gen_int_name_process(Channel_state* ch_state,
                                            char* fields)
{
    assert(ch_state != NULL);
    if (fields != NULL)
    {
        return false;
    }
    return set_active_name(&ch_state->parent, ACTIVE_CAT_CH_GEN,
                           ACTIVE_TYPE_INT, fields);
}


