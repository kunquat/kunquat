

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_GENERATOR_SET_TSTAMP_H
#define K_EVENT_GENERATOR_SET_TSTAMP_H


#include <Channel_state.h>
#include <Generator.h>
#include <Value.h>


bool Event_generator_set_tstamp_process(
        Generator* gen,
        Channel_state* ch_state,
        Value* value);


#endif // K_EVENT_GENERATOR_SET_TSTAMP_H


