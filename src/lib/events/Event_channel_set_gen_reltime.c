

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#include <Active_names.h>
#include <Event_common.h>
#include <Event_channel_set_gen_reltime.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


bool Event_channel_set_gen_reltime_process(Channel_state* ch_state,
                                           Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TIMESTAMP)
    {
        return false;
    }
    char* key = Active_names_get(ch_state->parent.active_names,
                                 ACTIVE_CAT_CH_GEN,
                                 ACTIVE_TYPE_TIMESTAMP);
    if (!string_has_suffix(key, ".jsont"))
    {
        return true;
    }
    return Channel_gen_state_modify_value(ch_state->cgstate, key,
                                          &value->value.Timestamp_type);
}


