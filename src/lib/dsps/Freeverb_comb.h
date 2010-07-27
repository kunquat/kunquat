

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


#ifndef K_FREEVERB_COMB_H
#define K_FREEVERB_COMB_H


#include <stdbool.h>
#include <stdint.h>

#include <frame.h>


/**
 * This is the lowpass-feedback-comb filter used by the Freeverb DSP.
 */
typedef struct Freeverb_comb Freeverb_comb;


/**
 * Creates a new Freeverb comb filter.
 *
 * \param buffer_size   The buffer size -- must be > \c 0.
 *
 * \return   The new Freeverb comb filter if successful, or \c NULL if memory
 *           allocation failed.
 */
Freeverb_comb* new_Freeverb_comb(uint32_t buffer_size);


/**
 * Sets the damp value of the Freeverb comb filter.
 *
 * \param comb   The Freeverb comb filter -- must not be \c NULL.
 * \param damp   The damp value -- must be >= \c 0 and <= \c 1.
 */
void Freeverb_comb_set_damp(Freeverb_comb* comb, kqt_frame damp);


/**
 * Sets the feedback of the Freeverb comb filter.
 *
 * \param comb       The Freeverb comb filter -- must not be \c NULL.
 * \param feedback   The feedback value -- must be > \c -1 and < \c 1.
 */
void Freeverb_comb_set_feedback(Freeverb_comb* comb, kqt_frame feedback);


/**
 * Processes one frame of input data.
 *
 * \param comb    The Freeverb comb filter -- must not be \c NULL.
 * \param input   The input frame.
 *
 * \return   The output frame.
 */
kqt_frame Freeverb_comb_process(Freeverb_comb* comb, kqt_frame input);


/**
 * Resizes the internal buffer of the Freeverb comb filter.
 *
 * \param comb       The Freeverb comb filter -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Freeverb_comb_resize_buffer(Freeverb_comb* comb, uint32_t new_size);


/**
 * Clears the internal buffer of the Freeverb comb filter.
 *
 * \param comb   The Freeverb comb filter -- must not be \c NULL.
 */
void Freeverb_comb_clear(Freeverb_comb* comb);


/**
 * Destroys an existing Freeverb comb filter.
 *
 * \param comb   The Freeverb comb filter, or \c NULL.
 */
void del_Freeverb_comb(Freeverb_comb* comb);


#endif // K_FREEVERB_COMB_H


