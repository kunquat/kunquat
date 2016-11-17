

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


#ifndef KQT_CONDITION_H
#define KQT_CONDITION_H


#include <threads/Mutex.h>

#ifdef WITH_PTHREAD
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


typedef struct Condition
{
    bool initialised;
    Mutex mutex;
#ifdef WITH_PTHREAD
    pthread_cond_t cond;
#endif
} Condition;


#define CONDITION_AUTO (&(Condition){ .initialised = false })


/**
 * Initialise the Condition.
 *
 * This function must not be called unless ENABLE_THREADS is defined.
 *
 * \param cond   The Condition -- must not be \c NULL and must be uninitialised.
 */
void Condition_init(Condition* cond);


/**
 * Get the initialisation status of the Condition.
 *
 * \param cond   The Condition -- must not be \c NULL.
 *
 * \return   \c true if \a cond is initialised, otherwise \c false.
 */
bool Condition_is_initialised(const Condition* cond);


/**
 * Get the Mutex associated with the Condition.
 *
 * \param cond   The Condition -- must not be \c NULL.
 */
Mutex* Condition_get_mutex(Condition* cond);


/**
 * Wait for the Condition to be signaled.
 *
 * The caller should lock the Mutex returned by \a Condition_get_mutex before
 * calling this function.
 *
 * \param cond   The Condition -- must not be \c NULL.
 */
void Condition_wait(Condition* cond);


/**
 * Wake up all threads waiting for the Condition.
 *
 * \param cond   The Condition -- must not be \c NULL.
 */
void Condition_broadcast(Condition* cond);


/**
 * Deinitialise the Condition.
 *
 * \param cond   The Condition -- must not be \c NULL and must not be waited
 *               for by any thread. Deinitialising an already uninitialised
 *               Condition is allowed.
 */
void Condition_deinit(Condition* cond);


#endif // KQT_CONDITION_H


