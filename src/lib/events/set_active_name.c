

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


#include <stdbool.h>

#include <Active_names.h>
#include <Event.h>
#include <Event_type.h>
#include <File_base.h>
#include <General_state.h>
#include <kunquat/limits.h>
#include <set_active_name.h>
#include <xassert.h>


Event_field_desc set_name_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


bool set_active_name(General_state* gstate,
                     Active_cat cat,
                     Active_type type,
                     char* fields)
{
    assert(gstate != NULL);
    assert(cat < ACTIVE_CAT_LAST);
    assert(type < ACTIVE_TYPE_LAST);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_name_desc, data, state);
    if (state->error)
    {
        return false;
    }
    char var_name[KQT_KEY_LENGTH_MAX] = "";
    state = READ_STATE_AUTO;
    read_string(data[0].field.string_type, var_name,
                KQT_KEY_LENGTH_MAX, state);
    if (state->error)
    {
        return false;
    }
    Active_names_set(gstate->active_names, cat, type, var_name);
    return true;
}


