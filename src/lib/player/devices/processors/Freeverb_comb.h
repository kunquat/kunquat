

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FREEVERB_COMB_H
#define KQT_FREEVERB_COMB_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * This is the lowpass-feedback-comb filter used by the Freeverb processor.
 */
typedef struct Freeverb_comb Freeverb_comb;


/**
 * Create a new Freeverb comb filter.
 *
 * \param buffer_size   The buffer size -- must be > \c 0.
 *
 * \return   The new Freeverb comb filter if successful, or \c NULL if memory
 *           allocation failed.
 */
Freeverb_comb* new_Freeverb_comb(int32_t buffer_size);


/**
 * Process data buffer.
 *
 * \param comb_l        The Freeverb comb filter -- must not be \c NULL.
 * \param out_buf       The output buffer -- must not be \c NULL.
 * \param in_buf        The input buffer -- must not be \c NULL.
 * \param refls         The reflectivity parameter buffer -- must not be \c NULL.
 * \param damps         The damp parameter buffer -- must not be \c NULL.
 * \param frame_count   Number of frames to be processed -- must be > \c 0.
 */
void Freeverb_comb_process(
        Freeverb_comb* comb,
        float* out_buf,
        const float* in_buf,
        const float* refls,
        const float* damps,
        int32_t frame_count);


/**
 * Resize the internal buffer of the Freeverb comb filter.
 *
 * \param comb       The Freeverb comb filter -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Freeverb_comb_resize_buffer(Freeverb_comb* comb, int32_t new_size);


/**
 * Clear the internal buffer of the Freeverb comb filter.
 *
 * \param comb   The Freeverb comb filter -- must not be \c NULL.
 */
void Freeverb_comb_clear(Freeverb_comb* comb);


/**
 * Destroy an existing Freeverb comb filter.
 *
 * \param comb   The Freeverb comb filter, or \c NULL.
 */
void del_Freeverb_comb(Freeverb_comb* comb);


#endif // KQT_FREEVERB_COMB_H


