

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
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
 * Change the amount of Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 * \param size   The new size -- must be > \c 0.
 *
 * \return   \c true if resizing succeeded, or \c false if memory allocation
 *           failed. Note that decreasing the number of Voices may still have
 *           occurred even if the operation fails.
 */
bool Voice_pool_resize(Voice_pool* pool, int size);


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
 * Get a Voice from the Voice pool.
 *
 * In case all the Voices are in use, the Voice considered least important is
 * reinitialised and returned.
 *
 * If the caller gives an existing Voice as a parameter, no new Voice will be
 * returned. Instead, the Voice pool will check whether this Voice has the
 * same ID as the caller provides (if yes, the caller is still allowed to use
 * this Voice and the \a voice parameter itself will be returned; otherwise
 * \c NULL).
 *
 * \param pool      The Voice pool -- must not be \c NULL.
 * \param voice     An existing Voice. If \c NULL is returned, the caller must
 *                  not access this Voice.
 * \param id        An identification number for an existing Voice that needs
 *                  to be matched.
 *
 * \return   The Voice reserved for use, or \c NULL if \a voice is no longer
 *           under the control of the caller or the pool size is \c 0.
 */
Voice* Voice_pool_get_voice(Voice_pool* pool, Voice* voice, uint64_t id);


void Voice_pool_sort_groups(Voice_pool* pool);


Voice_group* Voice_pool_get_group(
        const Voice_pool* pool, uint64_t group_id, Voice_group* vgroup);


/**
 * Start Voice group iteration.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
void Voice_pool_start_group_iteration(Voice_pool* pool);


/**
 * Get the next Voice group.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 *
 * \return   The next Voice group, or \c NULL if there are no groups left to
 *           be processed.
 */
Voice_group* Voice_pool_get_next_group(Voice_pool* pool);


#ifdef ENABLE_THREADS
/**
 * Get the next Voice group in a thread-safe way.
 *
 * \param pool     The Voice pool -- must not be \c NULL.
 * \param vgroup   Destination for the Voice group data -- must not be \c NULL.
 *
 * \return   The parameter \a vgroup, or \c NULL if there are no groups left to
 *           be processed.
 */
Voice_group* Voice_pool_get_next_group_synced(Voice_pool* pool, Voice_group* vgroup);
#endif


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


