

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


#ifndef K_TIME_ENV_STATE_H
#define K_TIME_ENV_STATE_H


#include <init/devices/param_types/Envelope.h>
#include <player/Work_buffer.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * A state for processing envelopes with a time axis.
 */
typedef struct Time_env_state
{
    bool is_finished;
    double cur_pos;
    int next_node_index;
    double cur_value;
    double update_value;
    double scale_factor;
} Time_env_state;


/**
 * Initialise the Time envelope state.
 *
 * \param testate   The Time envelope state -- must not be \c NULL.
 */
void Time_env_state_init(Time_env_state* testate);


/**
 * Process the given envelope.
 *
 * \param testate         The Time envelope state -- must not be \c NULL.
 * \param env             The Envelope -- must not be \c NULL.
 * \param has_loop        Whether the Envelope contains a loop.
 * \param scale_amount    The time scale amount -- must be finite.
 * \param scale_center    The time scale center -- must be finite.
 * \param sustain         Sustain value -- must be within range [0, 1]
 *                        (0 indicates no sustain).
 * \param min_value       Minimum envelope value -- must be finite.
 * \param max_value       Maximum envelope value -- must be finite.
 * \param pitch_wb        Input pitch values -- must not be \c NULL.
 * \param env_buf         Destination buffer for envelope values
 *                        -- must not be \c NULL.
 * \param buf_start       Write starting position of the work buffer
 *                        -- must be >= \c 0.
 * \param buf_stop        Write stopping position of the work buffer
 *                        -- must be >= \c 0.
 * \param audio_rate      The audio rate -- must be positive.
 *
 * \return   The stop index of the processing.
 */
int32_t Time_env_state_process(
        Time_env_state* testate,
        const Envelope* env,
        bool has_loop,
        double scale_amount,
        double scale_center,
        double sustain,
        double min_value,
        double max_value,
        const Work_buffer* pitch_wb,
        float* env_buf,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate);


#endif // K_TIME_ENV_STATE_H


