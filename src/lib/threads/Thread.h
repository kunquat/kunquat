

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


#ifndef KQT_THREAD_H
#define KQT_THREAD_H


#include <Error.h>

#ifdef WITH_PTHREAD
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


typedef struct Thread
{
    bool initialised;
#ifdef WITH_PTHREAD
    pthread_t thread;
#endif
    void* arg;
} Thread;


#define THREAD_AUTO (&(Thread){ .initialised = false })


/**
 * Initialise the Thread.
 *
 * This function must not be called unless ENABLE_THREADS is defined.
 *
 * \param thread   The Thread -- must not be \c NULL and must be uninitialised.
 * \param func     The function that \a thread executes -- must not be \c NULL.
 * \param arg      The argument passed to \a func.
 * \param error    Destination for error information -- must not be \c NULL.
 */
bool Thread_init(Thread* thread, void* (*func)(void*), void* arg, Error* error);


/**
 * Get the initialisation status of the Thread.
 *
 * \param thread   The Thread -- must not be \c NULL.
 *
 * \return   \c true if \a thread is initialised, otherwise \c false.
 */
bool Thread_is_initialised(const Thread* thread);


/**
 * Cancel the execution of the Thread.
 *
 * \param thread   The Thread -- must not be \c NULL.
 */
void Thread_cancel(Thread* thread);


/**
 * Join the Thread.
 *
 * \param thread   The Thread -- must not be \c NULL and must be initialised.
 */
void Thread_join(Thread* thread);


#endif // KQT_THREAD_H


