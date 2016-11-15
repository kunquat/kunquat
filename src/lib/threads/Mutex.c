

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


#include <threads/Mutex.h>

#include <debug/assert.h>

#ifdef WITH_PTHREAD
#include <errno.h>
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


void Mutex_init(Mutex* mutex)
{
    rassert(mutex != NULL);
    rassert(!mutex->initialised);

#ifndef ENABLE_THREADS
    rassert(false);
#endif

#ifdef WITH_PTHREAD
    const int status = pthread_mutex_init(&mutex->mutex, NULL);
    rassert(status == 0);
#else
    rassert(false);
#endif

    mutex->initialised = true;

    return;
}


void Mutex_lock(Mutex* mutex)
{
    rassert(mutex != NULL);
    rassert(mutex->initialised);

#ifdef WITH_PTHREAD
    const int status = pthread_mutex_lock(&mutex->mutex);
    rassert(status != EINVAL);
    rassert(status != EDEADLK);
    rassert(status == 0);
#endif

    return;
}


void Mutex_unlock(Mutex* mutex)
{
    rassert(mutex != NULL);
    rassert(mutex->initialised);

#ifdef WITH_PTHREAD
    const int status = pthread_mutex_unlock(&mutex->mutex);
    rassert(status != EINVAL);
    rassert(status != EPERM);
    rassert(status == 0);
#endif

    return;
}


void Mutex_deinit(Mutex* mutex)
{
    rassert(mutex != NULL);

    if (!mutex->initialised)
        return;

#ifdef WITH_PTHREAD
    const int status = pthread_mutex_destroy(&mutex->mutex);
    rassert(status != EBUSY);
#endif

    mutex->initialised = false;

    return;
}


