

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


#include <threads/Condition.h>

#include <debug/assert.h>
#include <threads/Mutex.h>

#ifdef WITH_PTHREAD
#include <errno.h>
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


void Condition_init(Condition* cond)
{
    rassert(cond != NULL);
    rassert(!cond->initialised);

#ifndef ENABLE_THREADS
    rassert(false);
#endif

    Mutex_init(&cond->mutex);

#ifdef WITH_PTHREAD
    const int status = pthread_cond_init(&cond->cond, NULL);
    rassert(status == 0);
#else
    rassert(false);
#endif

    cond->initialised = true;

    return;
}


bool Condition_is_initialised(const Condition* cond)
{
    rassert(cond != NULL);
    return cond->initialised;
}


Mutex* Condition_get_mutex(Condition* cond)
{
    rassert(cond != NULL);
    rassert(cond->initialised);

    return &cond->mutex;
}


void Condition_wait(Condition* cond)
{
    rassert(cond != NULL);
    rassert(cond->initialised);

#ifdef WITH_PTHREAD
    int status = pthread_cond_wait(&cond->cond, &cond->mutex.mutex);
    rassert(status == 0);
#endif

    return;
}


void Condition_broadcast(Condition* cond)
{
    rassert(cond != NULL);
    rassert(cond->initialised);

#ifdef WITH_PTHREAD
    int status = pthread_cond_broadcast(&cond->cond);
    rassert(status == 0);
#endif

    return;
}


void Condition_deinit(Condition* cond)
{
    rassert(cond != NULL);

    if (!cond->initialised)
        return;

#ifdef WITH_PTHREAD
    const int status = pthread_cond_destroy(&cond->cond);
    rassert(status != EBUSY);
#endif

    Mutex_deinit(&cond->mutex);

    cond->initialised = false;

    return;
}


