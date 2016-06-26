

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


#ifndef KQT_GAINCOMP_STATE_H
#define KQT_GAINCOMP_STATE_H


#include <decl.h>

#include <stdint.h>


Device_state* new_Gaincomp_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);


void Gaincomp_vstate_init(Voice_state* vstate, const Proc_state* proc_state);


#endif // KQT_GAINCOMP_STATE_H


