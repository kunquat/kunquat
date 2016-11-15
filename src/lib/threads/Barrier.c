

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


#include <threads/Barrier.h>

#include <debug/assert.h>
#include <Error.h>

#ifdef WITH_PTHREAD
#include <errno.h>
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


bool Barrier_init(Barrier* barrier, int count, Error* error)
{
    rassert(barrier != NULL);
    rassert(!barrier->initialised);
    rassert(count > 0);
    rassert(error != NULL);

#ifndef ENABLE_THREADS
    rassert(false);
#endif

    if (Error_is_set(error))
        return false;

#ifdef WITH_PTHREAD
    const int status =
        pthread_barrier_init(&barrier->barrier, NULL, (unsigned int)count);

    switch (status)
    {
        case 0:
            break;

        case EAGAIN:
        {
            Error_set(
                    error,
                    ERROR_RESOURCE,
                    "Could not allocate resources for synchronisation barrier");
            return false;
        }
        break;

        case EBUSY:
        case EINVAL:
        {
            rassert(false);
        }
        break;

        case ENOMEM:
        {
            Error_set(
                    error,
                    ERROR_MEMORY,
                    "Could not allocate memory for synchronisation barrier");
            return false;
        }
        break;

        default:
        {
            Error_set(
                    error,
                    ERROR_RESOURCE,
                    "Unexpected error code when creating synchronisation barrier: %d",
                    status);
            return false;
        }
        break;
    }

#else
    rassert(false);

#endif

    barrier->initialised = true;

    return true;
}


bool Barrier_wait(Barrier* barrier)
{
    rassert(barrier != NULL);
    rassert(barrier->initialised);

#ifdef WITH_PTHREAD
    const int status = pthread_barrier_wait(&barrier->barrier);
    rassert(status != EINVAL);

    if (status == PTHREAD_BARRIER_SERIAL_THREAD)
        return true;

    rassert(status == 0);
#endif

    return false;
}


void Barrier_deinit(Barrier* barrier)
{
    rassert(barrier != NULL);

    if (!barrier->initialised)
        return;

#ifdef WITH_PTHREAD
    const int status = pthread_barrier_destroy(&barrier->barrier);
    rassert(status != EBUSY);
    rassert(status != EINVAL);
    rassert(status == 0);
#endif

    barrier->initialised = false;

    return;
}


