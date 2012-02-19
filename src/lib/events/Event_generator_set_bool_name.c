

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
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
#include <Event_common.h>
#include <Event_generator_set_bool_name.h>
#include <File_base.h>
#include <Generator.h>
#include <set_active_name.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_generator,
                         EVENT_GENERATOR_SET_BOOL_NAME,
                         set_bool_name);


bool Event_generator_set_bool_name_process(Generator* gen,
                                           Channel_state* ch_state,
                                           Value* value)
{
    assert(gen != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)gen;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(&ch_state->parent, ACTIVE_CAT_GEN,
                           ACTIVE_TYPE_BOOL, value);
}


