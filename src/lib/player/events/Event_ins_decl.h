

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_INS_DECL_H
#define K_EVENT_INS_DECL_H


#include <stdbool.h>

#include <devices/Instrument_params.h>
#include <player/Ins_state.h>
#include <Value.h>


// Process function declarations

#define EVENT_INS_DEF(name, type_suffix, arg_type, validator) \
    bool Event_ins_##type_suffix##_process(                   \
            const Instrument_params* ins_params,              \
            Ins_state* ins_state,                             \
            const Value* value);
#include <player/events/Event_ins_types.h>


#endif // K_EVENT_INS_DECL_H


