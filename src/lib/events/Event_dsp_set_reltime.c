

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <limits.h>

#include <Active_names.h>
#include <Channel_state.h>
#include <DSP_conf.h>
#include <Event_common.h>
#include <Event_dsp_set_reltime.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>


bool Event_dsp_set_reltime_process(
        DSP_conf* dsp_conf,
        Channel_state* ch_state,
        Value* value)
{
    assert(dsp_conf != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    char* key = Active_names_get(
            ch_state->parent.active_names,
            ACTIVE_CAT_DSP,
            ACTIVE_TYPE_TSTAMP);
    if (!string_has_suffix(key, ".jsont"))
    {
        return false;
    }
    return Device_params_modify_value(
            dsp_conf->params,
            key,
            &value->value.Tstamp_type);
}


