

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_master_decl.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/events/Event_common.h>
#include <Value.h>

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_master_pattern_delay_process(
        Master_params* master_params, const Value* value)
{
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&master_params->delay_left, &value->value.Tstamp_type);
    return true;
}


