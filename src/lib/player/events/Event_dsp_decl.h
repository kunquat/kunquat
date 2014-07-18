

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


#ifndef K_EVENT_DSP_DECL_H
#define K_EVENT_DSP_DECL_H


#include <stdbool.h>

#include <devices/Device_impl.h>
#include <player/Channel.h>
#include <player/Device_state.h>
#include <Value.h>


// Process function declarations

#define EVENT_DSP_DEF(name, type_suffix, arg_type, validator) \
    bool Event_dsp_##type_suffix##_process(                   \
            const Device_impl* dimpl,                         \
            Device_state* dstate,                             \
            Channel* ch,                                      \
            const Value* value);
#include <player/events/Event_dsp_types.h>


#endif // K_EVENT_DSP_DECL_H


