

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


#ifndef KQT_EVENT_CHANNEL_DECL_H
#define KQT_EVENT_CHANNEL_DECL_H


#include <decl.h>
#include <player/Channel.h>
#include <player/Device_states.h>
#include <Value.h>

#include <stdbool.h>


// Process function declarations

#define EVENT_CHANNEL_DEF(name, type_suffix, arg_type, validator) \
    bool Event_channel_##type_suffix##_process(                   \
            Channel* ch,                                          \
            Device_states* dstates,                               \
            const Master_params* master_params,                   \
            const Value* value);
#include <player/events/Event_channel_types.h>


#endif // KQT_EVENT_CHANNEL_DECL_H


