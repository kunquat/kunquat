

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


#ifndef K_DSP_STATE_H
#define K_DSP_STATE_H


#include <stdlib.h>

#include <player/Device_state.h>


typedef struct DSP_state
{
    Device_state parent;
} DSP_state;


/**
 * Initialise the DSP state.
 *
 * \param dsp_state           The DSP state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 */
void DSP_state_init(
        DSP_state* dsp_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Reset the DSP state.
 *
 * \param dsp_state   The DSP state -- must not be \c NULL.
 */
void DSP_state_reset(DSP_state* dsp_state);


#endif // K_DSP_STATE_H


