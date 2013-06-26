

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
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

#include <Channel_state.h>
#include <DSP_conf.h>
#include <Value.h>


// Process function declarations

#define EVENT_TYPE_DEF(type)         \
    bool Event_dsp_##type##_process( \
            DSP_conf* dsp_conf,      \
            Channel_state* ch_state, \
            Value* value);
#include <events/Event_dsp_types.h>


#endif // K_EVENT_DSP_DECL_H


