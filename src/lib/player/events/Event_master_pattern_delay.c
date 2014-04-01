

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#include <kunquat/limits.h>
#include <player/events/Event_common.h>
#include <player/events/Event_master_decl.h>
#include <Value.h>
#include <xassert.h>


bool Event_master_pattern_delay_process(
        Master_params* master_params,
        Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&master_params->delay_left, &value->value.Tstamp_type);
    return true;
}


