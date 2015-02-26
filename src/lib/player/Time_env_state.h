

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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


#include <devices/param_types/Envelope.h>
#include <player/Work_buffers.h>


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
 * \param testate        The Time envelope state -- must not be \c NULL.
 * \param env            The Envelope -- must not be \c NULL.
 * \param scale_amount   The time scale amount -- must be finite.
 * \param scale_center   The time scale center -- must be finite.
 * \param min_value      Minimum envelope value -- must be finite.
 * \param max_value      Maximum envelope value -- must be finite.
 * \param wbs            The Work buffers -- must not be \c NULL. The envelope
 *                       values will be stored in \c WORK_BUFFER_TIME_ENV.
 * \param buf_start      Write starting position of the work buffer
 *                       -- must be >= \c 0.
 * \param buf_stop       Write stopping position of the work buffer
 *                       -- must be >= \c 0.
 * \param audio_rate     The audio rate -- must be positive.
 *
 * \return   The stop index of the processing.
 */
int32_t Time_env_state_process(
        Time_env_state* testate,
        const Envelope* env,
        double scale_amount,
        double scale_center,
        double min_value,
        double max_value,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate);


#endif // K_TIME_ENV_STATE_H


