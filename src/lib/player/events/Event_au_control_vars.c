

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_au_decl.h>

#include <debug/assert.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_au_set_cv_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(value != NULL);

    const char* var_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR);

    if (var_name == NULL)
        return false;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            &master_params->random,
            NULL,
            var_name,
            value);

    return true;
}


