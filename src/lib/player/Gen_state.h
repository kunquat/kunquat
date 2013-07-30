

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


#ifndef K_GEN_STATE_H
#define K_GEN_STATE_H


#include <player/Device_state.h>


typedef struct Gen_state
{
    Device_state parent;
} Gen_state;


/**
 * Initialises the Generator state.
 *
 * \param gs                  The Generator state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 */
void Gen_state_init(
        Gen_state* gs,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


#endif // K_GEN_STATE_H


