

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_STATE_H
#define KQT_PROC_STATE_H


#include <decl.h>
#include <init/devices/Proc_type.h>
#include <player/devices/Device_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef void Proc_state_clear_history_func(Proc_state*);

typedef void Proc_state_fire_event_func(Device_state*, const char*, const Value*);


struct Proc_state
{
    Device_state parent;

    bool is_voice_connected_to_mixed;

    Device_state_destroy_func* destroy;
    Device_state_set_audio_rate_func* set_audio_rate;
    Device_state_set_audio_buffer_size_func* set_audio_buffer_size;
    Device_state_set_tempo_func* set_tempo;
    Device_state_reset_func* reset;
    Device_state_render_mixed_func* render_mixed;

    Proc_state_clear_history_func* clear_history;
    Proc_state_fire_event_func* fire_dev_event;
};


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
 * Clear Processor state history.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 */
void Proc_state_clear_history(Proc_state* proc_state);


/**
 * Find out if the Processor state requires a Voice state in voice signal mode.
 */
bool Proc_state_needs_vstate(const Proc_state* proc_state);


#endif // KQT_PROC_STATE_H


