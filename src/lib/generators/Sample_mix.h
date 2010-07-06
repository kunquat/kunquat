

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <frame.h>
#include <Generator.h>
#include <Sample.h>
#include <Voice_state.h>


/**
 * Mixes a Sample.
 *
 * \param sample        The Sample -- must not be \c NULL.
 * \param gen           The Generator containing the Sample -- must not be
 *                      \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param nframes       The number of frames to be mixed.
 * \param offset        The buffer offset.
 * \param freq          The mixing frequency -- must be > \c 0.
 * \param tempo         The tempo -- must be > \c 0.
 * \param middle_tone   The frequency of the sound in the native speed of the
 *                      Sample -- must be > \c 0.
 * \param middle_freq   The mixing speed of the Sample used for playing
 *                      \a middle_tone -- must be > \c 0.
 */
uint32_t Sample_mix(Sample* sample,
                    Generator* gen,
                    Voice_state* state,
                    uint32_t nframes,
                    uint32_t offset,
                    uint32_t freq,
                    double tempo,
                    int buf_count,
                    kqt_frame** bufs,
                    double middle_tone,
                    double middle_freq);


#endif // K_SAMPLE_MIX_H


