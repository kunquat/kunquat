

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
#include <Event_common.h>
#include <Event_generator_set_reltime_name.h>
#include <File_base.h>
#include <Generator.h>
#include <set_active_name.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc* set_reltime_name_desc = set_name_desc;


Event_create_constructor(Event_generator,
                         EVENT_GENERATOR_SET_RELTIME_NAME,
                         set_reltime_name);


bool Event_generator_set_reltime_name_process(Generator* gen,
                                              Channel_state* ch_state,
                                              char* fields)
{
    assert(gen != NULL);
    (void)gen;
    if (fields == NULL)
    {
        return false;
    }
    return set_active_name(&ch_state->parent, ACTIVE_CAT_GEN,
                           ACTIVE_TYPE_TIMESTAMP, fields);
}


