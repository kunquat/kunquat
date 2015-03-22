

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_AU_DECL_H
#define K_EVENT_AU_DECL_H


#include <stdbool.h>

#include <devices/Audio_unit.h>
#include <devices/Au_params.h>
#include <player/Au_state.h>
#include <Value.h>


// Process function declarations

#define EVENT_AU_DEF(name, type_suffix, arg_type, validator) \
    bool Event_au_##type_suffix##_process(                   \
            const Audio_unit* au,                            \
            const Au_params* au_params,                      \
            Au_state* au_state,                              \
            Device_states* dstates,                          \
            const Value* value);
#include <player/events/Event_au_types.h>


#endif // K_EVENT_AU_DECL_H


