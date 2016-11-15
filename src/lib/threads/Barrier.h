

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


#ifndef KQT_BARRIER_H
#define KQT_BARRIER_H


#include <Error.h>

#ifdef WITH_PTHREAD
#endif

#include <stdbool.h>
#include <stdlib.h>


typedef struct Barrier
{
    bool initialised;

#ifdef WITH_PTHREAD
    pthread_barrier_t barrier;
#endif
} Barrier;


#define BARRIER_AUTO (&(Barrier){ .initialised = false })


/**
 * Initialise the Barrier.
 *
 * This function must not be called unless ENABLE_THREADS is defined.
 *
 * \param barrier   The Barrier -- must not be \c NULL and must be uninitialised.
 * \param count     The number of threads to wait at \a barrier --
 *                  must be > \c 0.
 * \param error     Destination for error information -- must not be \c NULL.
 */
bool Barrier_init(Barrier* barrier, int count, Error* error);


/**
 * Synchronise at the Barrier.
 *
 * \param barrier   The Barrier -- must not be \c NULL.
 *
 * \return   \c true for exactly one of the waiting threads, \c false for all
 *           the others.
 */
bool Barrier_wait(Barrier* barrier);


/**
 * Deinitialise the Barrier.
 *
 * \param barrier   The Barrier -- must not be \c NULL and must not be waited
 *                  at by any thread. Deinitialising an already uninitialised
 *                  Barrier is allowed.
 */
void Barrier_deinit(Barrier* barrier);


#endif // KQT_BARRIER_H


