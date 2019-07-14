

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


#ifndef KQT_VOICE_POOL_H
#define KQT_VOICE_POOL_H


#include <player/Voice.h>
#include <player/Voice_group.h>
#include <player/Voice_work_buffers.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


/**
 * Voice pool manages the allocation of Voices.
 */
typedef struct Voice_pool Voice_pool;


/**
 * Create a new Voice pool.
 *
 * \param size   The number of Voices in the Voice pool -- must be >= \c 0.
 *
 * \return   The new Voice pool if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice_pool* new_Voice_pool(int size);


/**
 * Reserve space for the Voice states.
 *
 * \param pool         The Voice pool -- must not be \c NULL.
 * \param state_size   The amount of bytes to reserve for the Voice states
 *                     -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_pool_reserve_state_space(Voice_pool* pool, int32_t state_size);


/**
 * Get the current Voice work buffer size.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 *
 * \return   The Voice work buffer size.
 */
int32_t Voice_pool_get_work_buffer_size(const Voice_pool* pool);


/**
 * Reserve Work buffers for Voices.
 *
 * \param pool       The Voice pool -- must not be \c NULL.
 * \param buf_size   The buffer size -- must be >= \c 0 and
 *                   < \c VOICE_WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_pool_reserve_work_buffers(Voice_pool* pool, int32_t buf_size);


/**
 * Get the amount of Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 *
 * \return   The number of Voices.
 */
int Voice_pool_get_size(const Voice_pool* pool);


/**
 * Get a new Voice group ID from the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 *
 * \return   The new group ID.
 */
uint64_t Voice_pool_new_group_id(Voice_pool* pool);


/**
 * Allocate a new Voice from the Voice pool.
 *
 * This function always succeeds. In case all the Voices are in use, an existing
 * Voice group (other than  one of given ID) will be reset and one of its Voices
 * is returned.
 *
 * \param pool          The Voice pool -- must not be \c NULL.
 * \param ch_num        The number of the channel requesting the Voice -- must be
 *                      >= \c 0 and < \c KQT_CHANNELS_MAX.
 * \param group_id      The Voice group ID of the new Voice -- must not be \c 0.
 * \param is_external   \c true if the allocation is used for an externally fired
 *                      event, otherwise \c false.
 *
 * \return   The new Voice.
 */
Voice* Voice_pool_allocate_voice(
        Voice_pool* pool, int ch_num, uint64_t group_id, bool is_external);


void Voice_pool_sort_fg_groups(Voice_pool* pool);


void Voice_pool_reset_group(Voice_pool* pool, uint64_t group_id);


/**
 * Get a foreground Voice group associated with a given channel.
 *
 * \param pool       The Voice pool -- must not be \c NULL.
 * \param ch_num     The channel number -- must be >= \c 0 and < \c KQT_CHANNELS_MAX.
 * \param group_id   The group ID.
 * \param vgroup     Destination address for the Voice group -- must not be \c NULL.
 *
 * \return   \a vgroup if a group matching \a ch_num and \a group_id was found,
 *           otherwise \c NULL.
 */
Voice_group* Voice_pool_get_fg_group(
        Voice_pool* pool, int ch_num, uint64_t group_id, Voice_group* vgroup);


/**
 * Start Voice group iteration.
 *
 * This function needs to be called before starting per-thread Voice group processing.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
void Voice_pool_start_group_iteration(Voice_pool* pool);


// Ok to call from a worker thread as long as no other thread handles ch_num
void Voice_pool_start_fg_ch_iteration(const Voice_pool* pool, int ch_num, int* ch_iter);


// Ok to call from a worker thread as long as no other thread handles ch_num
Voice_group* Voice_pool_get_next_fg_group(
        Voice_pool* pool, int ch_num, int* ch_iter, Voice_group* vgroup);


Voice_group* Voice_pool_get_next_bg_group(Voice_pool* pool, Voice_group* vgroup);


Voice_group* Voice_pool_get_next_bg_group_synced(Voice_pool* pool, Voice_group* vgroup);


/**
 * This is a light-weight clean-up of the Voice pool for cases where new note
 * events have been processed without a signal processing step.
 */
void Voice_pool_clean_up_fg_voices(Voice_pool* pool);


void Voice_pool_finish_group_iteration(Voice_pool* pool);


/**
 * Reset all Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
void Voice_pool_reset(Voice_pool* pool);


/**
 * Destroy an existing Voice pool.
 *
 * \param pool   The Voice pool, or \c NULL.
 */
void del_Voice_pool(Voice_pool* pool);


#endif // KQT_VOICE_POOL_H


