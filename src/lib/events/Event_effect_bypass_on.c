

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

#include <Effect.h>
#include <Event_common.h>
#include <Event_effect_bypass_on.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc bypass_on_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_effect,
                         EVENT_EFFECT_BYPASS_ON,
                         bypass_on);


bool Event_effect_bypass_on_process(Effect* eff, char* fields)
{
    assert(eff != NULL);
    (void)fields;
    Effect_set_bypass(eff, true);
    return true;
}


