

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
#include <math.h>

#include <Active_names.h>
#include <Channel_state.h>
#include <DSP_conf.h>
#include <Event_common.h>
#include <Event_dsp_set_float.h>
#include <File_base.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_float_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_dsp,
                         EVENT_DSP_SET_FLOAT,
                         set_float);


bool Event_dsp_set_float_process(DSP_conf* dsp_conf,
                                 Channel_state* ch_state,
                                 Value* value)
{
    assert(dsp_conf != NULL);
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    char* key = Active_names_get(ch_state->parent.active_names,
                                 ACTIVE_CAT_DSP,
                                 ACTIVE_TYPE_FLOAT);
    if (!string_has_suffix(key, ".jsonf"))
    {
        return true;
    }
    return Device_params_modify_value(dsp_conf->params, key,
                                      &value->value.float_type);
}


