

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SAMPLE_MIX_H
#define K_SAMPLE_MIX_H


#include <stdint.h>

#include <devices/Generator.h>
#include <devices/param_types/Sample.h>
#include <frame.h>
#include <player/Voice_state.h>
#include <player/Work_buffer.h>


/**
 * Mix a Sample.
 *
 * \param sample        The Sample -- must not be \c NULL.
 * \param params        The Sample parameters -- must not be \c NULL.
 * \param gen           The Generator containing the Sample -- must not be
 *                      \c NULL.
 * \param ins_state     The Instrument state -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param wbs           The Work buffers -- must not be \c NULL.
 * \param buf_start     The start index of the buffer area to be processed.
 * \param buf_stop      The stop index of the buffer area to be processed.
 * \param audio_rate    The audio rate -- must be positive.
 * \param tempo         The tempo -- must be > \c 0.
 * \param middle_tone   The frequency of the sound in the native speed of the
 *                      Sample -- must be > \c 0.
 * \param middle_freq   The mixing speed of the Sample used for playing
 *                      \a middle_tone -- must be > \c 0.
 * \param vol_scale     Volume scaling for this sample -- must be >= \c 0.
 */
uint32_t Sample_mix(
        const Sample* sample,
        const Sample_params* params,
        const Generator* gen,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo,
        double middle_tone,
        double middle_freq,
        double vol_scale);


#endif // K_SAMPLE_MIX_H


