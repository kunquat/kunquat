

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/Background_loader.h>

#include <debug/assert.h>
#include <Error.h>
#include <init/Background_loader.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <threads/Condition.h>
#include <threads/Mutex.h>
#include <threads/Thread.h>

#include <stdbool.h>
#include <stdlib.h>


#define QUEUE_SIZE 32


#define BACKGROUND_LOADER_TASK_AUTO \
    (&(Background_loader_task){ .process = NULL, .cleanup = NULL, .user_data = NULL })


typedef enum
{
    LOADER_STATE_WAITING,
    LOADER_STATE_IN_PROGRESS,
} Loader_state;


typedef enum
{
    TASK_INFO_EMPTY,
    TASK_INFO_READY_TO_START,
    TASK_INFO_IN_PROGRESS,
    TASK_INFO_FINISHED,
    TASK_INFO_FAILED,
} Task_info_state;


typedef struct Background_loader_task_info
{
    Background_loader_task task;
    Task_info_state state;
    Error error;
} Background_loader_task_info;


static Background_loader_task_info* Background_loader_fetch_task_info(
        Background_loader* loader);


static void Background_loader_set_task_finished(
        Background_loader* loader, Background_loader_task_info* task_info);


static void Background_loader_task_info_init(Background_loader_task_info* info)
{
    rassert(info != NULL);

    info->task = *BACKGROUND_LOADER_TASK_AUTO;
    info->state = TASK_INFO_EMPTY;
    info->error = *ERROR_AUTO;

    return;
}


static bool Background_loader_task_info_is_free(const Background_loader_task_info* info)
{
    rassert(info != NULL);
    return (info->state == TASK_INFO_EMPTY);
}


typedef struct Task_worker
{
    Thread thread;
    Background_loader* host;
} Task_worker;


static void Task_worker_run_tasks(Task_worker* worker)
{
    rassert(worker != NULL);

    Background_loader_task_info* task_info =
        Background_loader_fetch_task_info(worker->host);
    while (task_info != NULL)
    {
        rassert(task_info->state == TASK_INFO_IN_PROGRESS);
        rassert(task_info->task.process != NULL);

        task_info->task.process(&task_info->error, task_info->task.user_data);

        Background_loader_set_task_finished(worker->host, task_info);

        task_info = Background_loader_fetch_task_info(worker->host);
    }

    return;
}


static void* worker_thread(void* user_data)
{
    rassert(user_data != NULL);

    Task_worker* worker = user_data;
    Task_worker_run_tasks(worker);

    return NULL;
}


static void Task_worker_init(Task_worker* worker, Background_loader* host)
{
    rassert(worker != NULL);
    rassert(host != NULL);

    worker->thread = *THREAD_AUTO;
    worker->host = host;

    return;
}


static bool Task_worker_is_running(const Task_worker* worker)
{
    rassert(worker != NULL);
    return Thread_is_initialised(&worker->thread);
}


static bool Task_worker_start(Task_worker* worker, Error* error)
{
    rassert(worker != NULL);
    rassert(error != NULL);

    return Thread_init(&worker->thread, worker_thread, worker, error);
}


#ifdef ENABLE_THREADS
static void Task_worker_join(Task_worker* worker)
{
    rassert(worker != NULL);
    rassert(Thread_is_initialised(&worker->thread));

    Thread_join(&worker->thread);

    return;
}
#endif // ENABLE_THREADS


struct Background_loader
{
    int thread_count;
    Task_worker workers[KQT_THREADS_MAX];

    Error first_error;

    Mutex loader_lock;

    Condition signal;
    Loader_state state;

    int add_index;
    int fetch_index;
    Background_loader_task_info task_queue[QUEUE_SIZE];
};


Background_loader* new_Background_loader(void)
{
    Background_loader* loader = memory_alloc_item(Background_loader);
    if (loader == NULL)
        return NULL;

    loader->thread_count = 0;

    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        Task_worker_init(&loader->workers[i], loader);

    loader->first_error = *ERROR_AUTO;

    loader->loader_lock = *MUTEX_AUTO;
    loader->signal = *CONDITION_AUTO;

#ifdef ENABLE_THREADS
    Condition_init(&loader->signal);
    Mutex_init(&loader->loader_lock);
#endif

    loader->add_index = 0;
    loader->fetch_index = 0;

    for (int i = 0; i < QUEUE_SIZE; ++i)
        Background_loader_task_info_init(&loader->task_queue[i]);

    return loader;
}


bool Background_loader_set_thread_count(
        Background_loader* loader, int count, Error* error)
{
    rassert(loader != NULL);
    rassert(count >= 0);
    rassert(count <= KQT_THREADS_MAX);
    rassert(error != NULL);

#ifdef ENABLE_THREADS
    loader->thread_count = count;
#else
    loader->thread_count = 0;
#endif

    return true;
}


int Background_loader_get_thread_count(const Background_loader* loader)
{
    rassert(loader != NULL);
    return loader->thread_count;
}


static void Background_loader_set_task_finished(
        Background_loader* loader, Background_loader_task_info* task_info)
{
    rassert(loader != NULL);
    rassert(task_info != NULL);

    Mutex_lock(&loader->loader_lock);
    task_info->state = TASK_INFO_FINISHED;
    Mutex_unlock(&loader->loader_lock);

    return;
}


bool Background_loader_add_task(Background_loader* loader, Background_loader_task* task)
{
    rassert(loader != NULL);
    rassert(task != NULL);
    rassert(task->process != NULL);
    rassert(task->cleanup != NULL);

    if (loader->thread_count == 0)
        return false;

    // Get free task info
    Background_loader_task_info* selected_task_info = NULL;

    Mutex_lock(&loader->loader_lock);

    for (int i = 0; i < QUEUE_SIZE; ++i)
    {
        const int test_index = (loader->add_index + i) % QUEUE_SIZE;
        Background_loader_task_info* test_info = &loader->task_queue[test_index];

        if (Background_loader_task_info_is_free(test_info))
        {
            loader->add_index = (test_index + 1) % QUEUE_SIZE;

            selected_task_info = test_info;
            selected_task_info->task = *task;
            selected_task_info->state = TASK_INFO_READY_TO_START;
            selected_task_info->error = *ERROR_AUTO;

            break;
        }
    }

    Mutex_unlock(&loader->loader_lock);

    // Start workers if not running yet
    {
        bool any_workers_running = false;
        for (int i = 0; i < loader->thread_count; ++i)
        {
            Error* error = ERROR_AUTO;
            if (Task_worker_is_running(&loader->workers[i]))
            {
                any_workers_running = true;
            }
            else
            {
                Task_worker_start(&loader->workers[i], error);
                if (!Error_is_set(error))
                    any_workers_running = true;
            }
        }

        if (!any_workers_running)
        {
            // Clear the task info as there are no workers that could run it
            Background_loader_task_info_init(selected_task_info);
            return false;
        }
    }

    // Signal threads that there is more work to be done
    Mutex* signal_mutex = Condition_get_mutex(&loader->signal);
    Mutex_lock(signal_mutex);
    loader->state = LOADER_STATE_IN_PROGRESS;
    Condition_broadcast(&loader->signal);
    Mutex_unlock(signal_mutex);

    return true;
}


static Background_loader_task_info* Background_loader_fetch_task_info(
        Background_loader* loader)
{
    rassert(loader != NULL);

    Background_loader_task_info* selected_task_info = NULL;
    bool all_done = false;

    while ((selected_task_info == NULL) && !all_done)
    {
        Mutex_lock(&loader->loader_lock);

        for (int i = 0; i < QUEUE_SIZE; ++i)
        {
            const int test_index = (loader->fetch_index + i) % QUEUE_SIZE;
            Background_loader_task_info* test_info = &loader->task_queue[test_index];

            if (test_info->state == TASK_INFO_READY_TO_START)
            {
                loader->fetch_index = (test_index + 1) % QUEUE_SIZE;

                selected_task_info = test_info;
                selected_task_info->state = TASK_INFO_IN_PROGRESS;
                selected_task_info->error = *ERROR_AUTO;
            }
        }

        if (selected_task_info == NULL)
        {
            if (loader->state == LOADER_STATE_WAITING)
            {
                all_done = true;
            }
            else
            {
                Mutex_unlock(&loader->loader_lock);

                // Wait for another signal
                Mutex* signal_mutex = Condition_get_mutex(&loader->signal);
                Mutex_lock(signal_mutex);
                Condition_wait(&loader->signal);
                Mutex_unlock(signal_mutex);

                continue;
            }
        }

        Mutex_unlock(&loader->loader_lock);
    }

    return selected_task_info;
}


void Background_loader_run_cleanups(Background_loader* loader)
{
    rassert(loader != NULL);

#ifdef ENABLE_THREADS
    Mutex_lock(&loader->loader_lock);

    for (int i = 0; i < QUEUE_SIZE; ++i)
    {
        Background_loader_task_info* task_info = &loader->task_queue[i];
        if (task_info->state == TASK_INFO_FINISHED)
        {
            rassert(task_info->task.cleanup != NULL);
            task_info->task.cleanup(&task_info->error, task_info->task.user_data);

            if (!Error_is_set(&task_info->error))
                Background_loader_task_info_init(task_info);
            else
                task_info->state = TASK_INFO_FAILED;
        }
    }

    Mutex_unlock(&loader->loader_lock);
#endif // ENABLE_THREADS

    return;
}


void Background_loader_wait_idle(Background_loader* loader)
{
    rassert(loader != NULL);

#ifdef ENABLE_THREADS
    // Let all tasks know that no more tasks are coming
    Mutex* signal_mutex = Condition_get_mutex(&loader->signal);
    Mutex_lock(signal_mutex);
    loader->state = LOADER_STATE_WAITING;
    Condition_broadcast(&loader->signal);
    Mutex_unlock(signal_mutex);

    // Join all threads
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        if (Task_worker_is_running(&loader->workers[i]))
            Task_worker_join(&loader->workers[i]);
    }

    Background_loader_run_cleanups(loader);
#endif // ENABLE_THREADS

    return;
}


const Error* Background_loader_get_first_error(const Background_loader* loader)
{
    rassert(loader != NULL);

    if (Error_is_set(&loader->first_error))
        return &loader->first_error;

    return NULL;
}


void Background_loader_reset(Background_loader* loader)
{
    rassert(loader != NULL);

#ifdef ENABLE_THREADS
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        rassert(!Task_worker_is_running(&loader->workers[i]));
#endif

    return;
}


void del_Background_loader(Background_loader* loader)
{
    if (loader == NULL)
        return;

    memory_free(loader);

    return;
}


