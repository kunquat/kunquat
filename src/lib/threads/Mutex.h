

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


#ifndef KQT_MUTEX_H
#define KQT_MUTEX_H


#ifdef WITH_PTHREAD
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


typedef struct Mutex
{
    bool initialised;
#ifdef WITH_PTHREAD
    pthread_mutex_t mutex;
#endif
} Mutex;


#define MUTEX_AUTO (&(Mutex){ .initialised = false })


/**
 * Initialise the Mutex.
 *
 * This function must not be called unless ENABLE_THREADS is defined.
 *
 * \param mutex   The Mutex -- must not be \c NULL and must be uninitialised.
 */
void Mutex_init(Mutex* mutex);


/**
 * Lock the Mutex.
 *
 * \param mutex   The Mutex -- must not be \c NULL and must not be locked by
 *                the calling thread.
 */
void Mutex_lock(Mutex* mutex);


/**
 * Unlock the Mutex.
 *
 * \param mutex   The Mutex -- must not be \c NULL and must be locked by the
 *                calling thread.
 */
void Mutex_unlock(Mutex* mutex);


/**
 * Deinitialise the Mutex.
 *
 * \param mutex   The Mutex -- must not be \c NULL and must be unlocked.
 *                Deinitialising an already uninitialised Mutex is allowed.
 */
void Mutex_deinit(Mutex* mutex);


#endif // KQT_MUTEX_H


