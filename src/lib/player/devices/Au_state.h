

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


#ifndef K_AU_STATE_H
#define K_AU_STATE_H


#include <Decl.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Au_state
{
    Device_state parent;

    bool bypass;
    double sustain; // 0 = no sustain, 1.0 = full sustain
    Device_states* dstates; // required for rendering, TODO: make less hacky
};


/**
 * Initialise the Audio unit state.
 *
 * \param au_state            The Audio unit state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 */
void Au_state_init(
        Au_state* au_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Set Device states for the Audio unit state.
 *
 * \param au_state   The Audio unit state -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 */
void Au_state_set_device_states(Au_state* au_state, Device_states* dstates);


#endif // K_AU_STATE_H


