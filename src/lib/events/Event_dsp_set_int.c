

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
#include <Channel_state.h>
#include <DSP_conf.h>
#include <Event_common.h>
#include <Event_dsp_set_int.h>
#include <File_base.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_dsp,
                         EVENT_DSP_SET_INT,
                         set_int);


bool Event_dsp_set_int_process(DSP_conf* dsp_conf,
                               Channel_state* ch_state,
                               Value* value)
{
    assert(dsp_conf != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    char* key = Active_names_get(ch_state->parent.active_names,
                                 ACTIVE_CAT_DSP,
                                 ACTIVE_TYPE_INT);
    if (!string_has_suffix(key, ".jsoni"))
    {
        return true;
    }
    return Device_params_modify_value(dsp_conf->params, key,
                                      &value->value.int_type);
}


