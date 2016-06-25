

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_GENERAL_DECL_H
#define KQT_EVENT_GENERAL_DECL_H


#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>


// Process function declarations

#define EVENT_GENERAL_DEF(name, type_suffix, arg_type, validator) \
    bool Event_general_##type_suffix##_process(                   \
            General_state* gstate, const Value* value);
#include <player/events/Event_general_types.h>


#endif // KQT_EVENT_GENERAL_DECL_H


