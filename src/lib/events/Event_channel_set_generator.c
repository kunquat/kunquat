

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
#include <stdbool.h>

#include <Channel.h>
#include <Channel_state.h>
#include <Event_common.h>
#include <Event_channel_set_generator.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


bool Event_channel_set_generator_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    ch_state->generator = value->value.int_type;
    return true;
}


