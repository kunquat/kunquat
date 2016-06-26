

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_JUMP_CACHE_H
#define KQT_JUMP_CACHE_H


#include <containers/AAtree.h>

#include <stdlib.h>


/**
 * A pool of Jump contexts.
 */
typedef struct Jump_cache Jump_cache;


/**
 * Create a new Jump cache.
 *
 * \param num_contexts   The number of Jump contexts in the cache
 *                       -- must be > \c 0.
 *
 * \return   The new Jump cache if successful, or \c NULL if memory
 *           allocation failed.
 */
Jump_cache* new_Jump_cache(size_t num_contexts);


/**
 * Get a Jump context handle from the Jump cache.
 *
 * \param jcache   The Jump cache -- must not be \c NULL.
 *
 * \return   The Jump context handle, or \c NULL if \a jcache is empty.
 */
AAnode* Jump_cache_acquire_context(Jump_cache* jcache);


/**
 * Release a Jump context handle belonging to the Jump cache.
 *
 * \param jcache   The Jump cache -- must not be \c NULL.
 * \param handle   The Jump context handle -- must not be \c NULL.
 */
void Jump_cache_release_context(Jump_cache* jcache, AAnode* handle);


/**
 * Destroy an existing Jump cache.
 *
 * \param jcache   The Jump cache, or \c NULL. If not \c NULL, \a jcache
 *                 must contain all the Jump contexts it manages.
 */
void del_Jump_cache(Jump_cache* jcache);


#endif // KQT_JUMP_CACHE_H


