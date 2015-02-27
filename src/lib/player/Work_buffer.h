

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


#ifndef K_WORK_BUFFER_H
#define K_WORK_BUFFER_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <kunquat/limits.h>


#define WORK_BUFFER_SIZE_MAX ((KQT_AUDIO_BUFFER_SIZE_MAX) + 1)


/**
 * A buffer of float values for devices to use as temporary storage.
 */
typedef struct Work_buffer Work_buffer;


/**
 * Create a new Work buffer.
 *
 * \param size   The buffer size -- must be >= \c 0 and
 *               <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   The new Work buffer if successful, or \c NULL if memory allocation
 *           failed.
 */
Work_buffer* new_Work_buffer(uint32_t size);


/**
 * Resize the Work buffer.
 *
 * \param buffer     The Work buffer -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be >= \c 0 and
 *                   <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Work_buffer_resize(Work_buffer* buffer, uint32_t new_size);


/**
 * Get the size of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The size of the buffer.
 */
uint32_t Work_buffer_get_size(const Work_buffer* buffer);


/**
 * Get the contents of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The address of the internal buffer, with -1 as the first valid
 *           index. For devices that receive the buffer from a caller, this
 *           function never returns \c NULL.
 */
const float* Work_buffer_get_contents(const Work_buffer* buffer);


/**
 * Get the mutable contents of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The address of the internal buffer, with -1 as the first valid
 *           index. For devices that receive the buffer from a caller, this
 *           function never returns \c NULL.
 */
float* Work_buffer_get_contents_mut(const Work_buffer* buffer);


/**
 * Copy contents of the Work buffer into another.
 *
 * \param dest    The destination Work buffer -- must not be \c NULL.
 * \param src     The source Work buffer -- must not be \c NULL or \a dest.
 * \param start   Start index of copying -- must not exceed buffer length.
 * \param stop    Stop index of copying -- must not exceed buffer length.
 */
void Work_buffer_copy(
        const Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        uint32_t start,
        uint32_t stop);


/**
 * Destroy an existing Work buffer.
 *
 * \param buffer   The Work buffer, or \c NULL.
 */
void del_Work_buffer(Work_buffer* buffer);


#endif // K_WORK_BUFFER_H


