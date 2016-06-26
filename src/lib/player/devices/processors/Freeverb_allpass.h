

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FREEVERB_ALLPASS_H
#define KQT_FREEVERB_ALLPASS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * This is the "allpass" filter used by the Freeverb processor. It is not
 * really an allpass filter at most settings, but the name is kept in line with
 * the original Freeverb implementation.
 */
typedef struct Freeverb_allpass Freeverb_allpass;


/**
 * Create a new Freeverb allpass filter.
 *
 * \param buffer_size   The buffer size -- must be > \c 0.
 *
 * \return   The new Freeverb allpass filter if successful, or \c NULL if
 *           memory allocation failed.
 */
Freeverb_allpass* new_Freeverb_allpass(uint32_t buffer_size);


/**
 * Set the feedback of the Freeverb allpass filter.
 *
 * \param allpass    The Freeverb allpass filter -- must not be \c NULL.
 * \param feedback   The feedback value -- must be > \c -1 and < \c 1.
 */
void Freeverb_allpass_set_feedback(Freeverb_allpass* allpass, float feedback);


/**
 * Process data buffer.
 *
 * \param allpass     The Freeverb allpass filter -- must not be \c NULL.
 * \param buffer      The input/output buffer -- must not be \c NULL.
 * \param buf_start   The buffer start position -- must be >= \c 0.
 * \param buf_stop    The buffer stop position -- must be > \a buf_start.
 */
void Freeverb_allpass_process(
        Freeverb_allpass* allpass, float* buffer, int32_t buf_start, int32_t buf_stop);


/**
 * Resize the internal buffer of the Freeverb allpass filter.
 *
 * \param allpass    The Freeverb allpass filter -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Freeverb_allpass_resize_buffer(Freeverb_allpass* allpass, uint32_t new_size);


/**
 * Clear the internal buffer of the Freeverb allpass filter.
 *
 * \param allpass   The Freeverb allpass filter -- must not be \c NULL.
 */
void Freeverb_allpass_clear(Freeverb_allpass* allpass);


/**
 * Destroy an existing Freeverb allpass filter.
 *
 * \param allpass   The Freeverb allpass filter, or \c NULL.
 */
void del_Freeverb_allpass(Freeverb_allpass* allpass);


#endif // KQT_FREEVERB_ALLPASS_H


