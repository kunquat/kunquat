

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


#ifndef KQT_SAMPLE_STATE_H
#define KQT_SAMPLE_STATE_H


#include <decl.h>
#include <player/devices/Voice_state.h>

#include <stdlib.h>


Voice_state_get_size_func Sample_vstate_get_size;

void Sample_vstate_init(Voice_state* vstate, const Proc_state* proc_state);


#endif // KQT_SAMPLE_STATE_H


