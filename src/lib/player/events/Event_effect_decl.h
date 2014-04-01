

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


#ifndef K_EVENT_EFFECT_DECL_H
#define K_EVENT_EFFECT_DECL_H


#include <stdbool.h>

#include <devices/Effect.h>
#include <player/Device_states.h>
#include <player/Effect_state.h>
#include <Value.h>


// Process function declarations

#define EVENT_TYPE_DEF(type)            \
    bool Event_effect_##type##_process( \
            const Effect* eff,          \
            Effect_state* eff_state,    \
            Device_states* dstates,     \
            Value* value);
#include <player/events/Event_effect_types.h>


#endif // K_EVENT_EFFECT_DECL_H


