

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_VOICE_WORK_BUFFERS_H
#define KQT_VOICE_WORK_BUFFERS_H


#include <decl.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define VOICE_WORK_BUFFER_SIZE_MAX 1048576


typedef struct Voice_work_buffers Voice_work_buffers;


/**
 * Create new Voice work buffers.
 *
 * \return   The new Voice work buffers if successful, or \c NULL if memory
 *           allocation failed.
 */
Voice_work_buffers* new_Voice_work_buffers(void);


/**
 * Get the buffer size of a Work buffer inside Voice work buffers.
 *
 * \param wbs   The Voice work buffers -- must not be \c NULL.
 *
 * \return   The size of a single Work buffer, or \c 0 if \a wbs is uninitialised.
 */
int32_t Voice_work_buffers_get_buffer_size(const Voice_work_buffers* wbs);


/**
 * Allocate space for Voice work buffers.
 *
 * \param wbs        The Voice work buffers -- must not be \c NULL.
 * \param count      The number of buffers -- must be >= \c 0 and
 *                   <= \c KQT_VOICES_MAX.
 * \param buf_size   The new buffer size -- must be >= \c 0 and
 *                   <= \c VOICE_WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_work_buffers_allocate_space(
        Voice_work_buffers* wbs, int count, int32_t buf_size);


/**
 * Get a Work buffer in Voice work buffers.
 *
 * NOTE: \a Voice_work_buffers_allocate_space must be called before using this
 *       function.
 *
 * \param wbs     The Voice work buffers -- must not be \c NULL.
 * \param index   The index of the buffer -- must be >= \c 0 and less than
 *                the number of buffers inside \a wbs.
 *
 * \return   The Work buffer at \a index, or \c NULL if no buffer space has
 *           been allocated.
 */
Work_buffer* Voice_work_buffers_get_buffer_mut(const Voice_work_buffers* wbs, int index);


/**
 * Destroy existing Voice work buffers.
 *
 * \param wbs   The Voice work buffers, or \c NULL.
 */
void del_Voice_work_buffers(Voice_work_buffers* wbs);


#endif // KQT_VOICE_WORK_BUFFERS_H


