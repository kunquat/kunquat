

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_AUDIO_BUFFER_H
#define K_AUDIO_BUFFER_H


#include <stdbool.h>
#include <stdint.h>

#include <frame.h>


typedef struct Audio_buffer Audio_buffer;


/**
 * Create a new Audio buffer.
 *
 * \param size   The buffer size -- must be >= \c 0 and
 *               <= \c KQT_AUDIO_BUFFER_SIZE_MAX.
 *
 * \return   The new buffer if successful, or \c NULL if memory allocation
 *           failed.
 */
Audio_buffer* new_Audio_buffer(uint32_t size);


/**
 * Get the size of the Audio buffer.
 *
 * \param buffer   The Audio buffer -- must not be \c NULL.
 *
 * \return   The size of the Audio buffer.
 */
uint32_t Audio_buffer_get_size(const Audio_buffer* buffer);


/**
 * Resize the Audio buffer.
 *
 * \param buffer   The Audio buffer -- must not be \c NULL.
 * \param size     The new buffer size -- must be >= \c 0 and
 *                 <= \c KQT_AUDIO_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Audio_buffer_resize(Audio_buffer* buffer, uint32_t size);


/**
 * Clear the Audio buffer.
 *
 * \param buffer   The Audio buffer -- must not be \c NULL.
 * \param start    The first frame to be cleared -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be cleared -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be cleared.
 */
void Audio_buffer_clear(Audio_buffer* buffer, uint32_t start, uint32_t until);


/**
 * Mix the contents of an Audio buffer into another.
 *
 * If the two buffers are the same Audio buffer, this function does nothing.
 *
 * \param buffer   The Audio buffer that will contain the end result -- must
 *                 not be \c NULL.
 * \param in       The input Audio buffer -- must not be \c NULL and must
 *                 have the same size as \a buffer.
 * \param start    The first frame to be mixed -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be mixed -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be mixed.
 */
void Audio_buffer_mix(
        Audio_buffer* buffer,
        const Audio_buffer* in,
        uint32_t start,
        uint32_t until);


/**
 * Return an internal buffer of the Audio buffer.
 *
 * \param buffer    The Audio buffer -- must not be \c NULL.
 * \param channel   The buffer channel number -- must be >= \c 0 and
 *                  < \c KQT_BUFFERS_MAX.
 *
 * \return   The internal buffer. This value must not be cached by the caller.
 */
kqt_frame* Audio_buffer_get_buffer(Audio_buffer* buffer, int index);


/**
 * Destroy an existing Audio buffer.
 *
 * \param buffer   The Audio buffer, or \c NULL.
 */
void del_Audio_buffer(Audio_buffer* buffer);


#endif // K_AUDIO_BUFFER_H


