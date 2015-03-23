

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


#ifndef K_PROC_STATE_H
#define K_PROC_STATE_H


#include <stdbool.h>
#include <stdlib.h>

#include <Audio_buffer.h>
#include <player/Device_state.h>


typedef struct Proc_state
{
    Device_state parent;

    Audio_buffer* in_voice_buf;
    Audio_buffer* out_voice_buf;
} Proc_state;


/**
 * Initialise the Processor state.
 *
 * \param proc_state          The Processor state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Reset the Processor state.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 */
void Proc_state_reset(Proc_state* proc_state);


/**
 * Resize buffers of the Processor state.
 *
 * \param dstate     The Processor state -- must not be \c NULL.
 * \param new_size   The new buffer size -- must not be negative.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Proc_state_resize_buffers(Device_state* dstate, int32_t new_size);


/**
 * Deinitialises the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 */
void Proc_state_deinit(Device_state* dstate);


#endif // K_PROC_STATE_H


