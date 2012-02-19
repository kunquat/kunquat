

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

#include <Channel_state.h>
#include <DSP_conf.h>
#include <Event_common.h>
#include <Event_dsp_set_int_name.h>
#include <File_base.h>
#include <set_active_name.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_dsp,
                         EVENT_DSP_SET_INT_NAME,
                         set_int_name);


bool Event_dsp_set_int_name_process(DSP_conf* dsp_conf,
                                    Channel_state* ch_state,
                                    Value* value)
{
    assert(dsp_conf != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    (void)dsp_conf;
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(&ch_state->parent, ACTIVE_CAT_DSP,
                           ACTIVE_TYPE_INT, value);
}


