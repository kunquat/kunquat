

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <DSP_conf.h>
#include <Event_common.h>
#include <Event_dsp_set_int.h>
#include <File_base.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_int_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = INT64_MIN,
        .max.field.integral_type = INT64_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_dsp_set_int,
                         EVENT_DSP_SET_INT,
                         set_int_desc);


bool Event_dsp_set_int_process(DSP_conf* dsp_conf, char* fields)
{
    assert(dsp_conf != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Read_state* state = READ_STATE_AUTO;
    fields = read_const_char(fields, '[', state);
    char key[100] = { '\0' };
    fields = read_string(fields, key, 99, state);
    fields = read_const_char(fields, ',', state);
    if (state->error || !string_has_suffix(key, ".jsoni"))
    {
        return false;
    }
    return Device_params_modify_value(dsp_conf->params, key, fields);
}


