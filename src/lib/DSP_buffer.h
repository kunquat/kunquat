

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


#ifndef K_DSP_BUFFER_H
#define K_DSP_BUFFER_H


#include <stdbool.h>
#include <stdint.h>

#include <frame.h>


typedef struct DSP_buffer DSP_buffer;


/**
 * Creates a new DSP buffer.
 *
 * \param size   The buffer size -- must be > \c 0 and <= \c 4194304.
 *
 * \return   The new buffer if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP_buffer* new_DSP_buffer(uint32_t size);


/**
 * Gets the size of the DSP buffer.
 *
 * \param buffer   The DSP buffer -- must not be \c NULL.
 *
 * \return   The size of the DSP buffer.
 */
uint32_t DSP_buffer_get_size(DSP_buffer* buffer);


/**
 * Resizes the DSP buffer.
 *
 * \param buffer   The DSP buffer -- must not be \c NULL.
 * \param size     The new buffer size -- must be > \c 0 and <= \c 4194304.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool DSP_buffer_resize(DSP_buffer* buffer, uint32_t size);


/**
 * Clears the DSP buffer.
 *
 * \param buffer   The DSP buffer -- must not be \c NULL.
 */
void DSP_buffer_clear(DSP_buffer* buffer);


/**
 * Mixes the contents of a DSP buffer into another.
 *
 * If the two buffers are the same DSP buffer, this function does nothing.
 *
 * \param buffer   The DSP buffer that will contain the end result -- must
 *                 not be \c NULL.
 * \param in       The input DSP buffer -- must not be \c NULL and must
 *                 have the same size as \a buffer.
 */
void DSP_buffer_mix(DSP_buffer* buffer, DSP_buffer* in);


/**
 * Returns an internal buffer of the DSP buffer.
 *
 * \param buffer    The DSP buffer -- must not be \c NULL.
 * \param channel   The buffer channel number -- must be >= \c 0 and
 *                  < \c KQT_BUFFERS_MAX.
 *
 * \return   The internal buffer. This value must not be cached by the caller.
 */
kqt_frame* DSP_buffer_get_buffer(DSP_buffer* buffer, int index);


/**
 * Destroys an existing DSP buffer.
 *
 * \param buffer   The DSP buffer -- must not be \c NULL.
 */
void del_DSP_buffer(DSP_buffer* buffer);


#endif // K_DSP_BUFFER_H


