

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
    LOADER_STATE_INIT,
    LOADER_STATE_IN_PROGRESS,
    LOADER_STATE_WAITING,
} Loader_state;


typedef enum
{
    TASK_INFO_EMPTY,
    TASK_INFO_END_QUEUE,
    TASK_INFO_READY_TO_START,
    TASK_INFO_IN_PROGRESS,
    TASK_INFO_FINISHED,
    TASK_INFO_FAILED,
} Task_info_state;


typedef struct Task_info
{
    Background_loader_task task;
    Task_info_state state;
    Error error;
} Task_info;


#define TASK_INFO_AUTO (&(Task_info){           \
        .task = *BACKGROUND_LOADER_TASK_AUTO,   \
        .state = TASK_INFO_EMPTY,               \
        .error = *ERROR_AUTO,                   \
    })


static bool Background_loader_fetch_task_info(
        Background_loader* loader, Task_info* dest_task_info);


static void Background_loader_add_cleanup_task_info(
        Background_loader* loader, Task_info* task_info);


static void Task_info_init(Task_info* info)
{
    rassert(info != NULL);

    info->task = *BACKGROUND_LOADER_TASK_AUTO;
    info->state = TASK_INFO_EMPTY;
    info->error = *ERROR_AUTO;

    return;
}


typedef struct Task_worker
{
    Thread thread;
    Background_loader* host;
    Task_info task_info;
} Task_worker;


static void Task_worker_run_tasks(Task_worker* worker)
{
    rassert(worker != NULL);

    bool task_found =
        Background_loader_fetch_task_info(worker->host, &worker->task_info);
    while (task_found)
    {
        rassert(worker->task_info.state == TASK_INFO_IN_PROGRESS);
        rassert(worker->task_info.task.process != NULL);

        worker->task_info.task.process(
                &worker->task_info.error, worker->task_info.task.user_data);

        Background_loader_add_cleanup_task_info(worker->host, &worker->task_info);

        task_found = Background_loader_fetch_task_info(worker->host, &worker->task_info);
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


static bool Task_worker_is_running(const Task_worker* worker)
{
    rassert(worker != NULL);
    return Thread_is_initialised(&worker->thread);
}


static void Task_worker_init(Task_worker* worker, Background_loader* host)
{
    rassert(worker != NULL);
    rassert(host != NULL);

    worker->thread = *THREAD_AUTO;
    worker->host = host;
    Task_info_init(&worker->task_info);

    return;
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


static void Task_worker_deinit(Task_worker* worker)
{
    rassert(worker != NULL);
    return;
}


typedef struct Task_queue
{
    Mutex lock;
    int add_index;
    int fetch_index;
    Task_info tasks[QUEUE_SIZE];
} Task_queue;


static void Task_queue_init(Task_queue* queue)
{
    rassert(queue != NULL);

    queue->lock = *MUTEX_AUTO;

#ifdef ENABLE_THREADS
    Mutex_init(&queue->lock);
#endif

    queue->add_index = 0;
    queue->fetch_index = 0;

    for (int i = 0; i < QUEUE_SIZE; ++i)
        Task_info_init(&queue->tasks[i]);

    return;
}


static bool Task_queue_add(Task_queue* queue, Task_info* task_info)
{
    rassert(queue != NULL);
    rassert(task_info != NULL);

    Mutex_lock(&queue->lock);

    bool is_full =
        (((queue->add_index + 1) % QUEUE_SIZE) == queue->fetch_index);

    if (task_info->state != TASK_INFO_END_QUEUE)
        is_full |= (((queue->add_index + 2) % QUEUE_SIZE) == queue->fetch_index);

    if (is_full)
    {
        Mutex_unlock(&queue->lock);
        return false;
    }

    queue->tasks[queue->add_index] = *task_info;
    queue->add_index = (queue->add_index + 1) % QUEUE_SIZE;

    Mutex_unlock(&queue->lock);

    return true;
}


static void Task_queue_undo_add(Task_queue* queue)
{
    rassert(queue != NULL);

    Mutex_lock(&queue->lock);

    rassert(queue->fetch_index != queue->add_index);
    queue->add_index = (queue->add_index + QUEUE_SIZE - 1) % QUEUE_SIZE;
    Task_info_init(&queue->tasks[queue->add_index]);

    Mutex_unlock(&queue->lock);

    return;
}


static bool Task_queue_fetch(Task_queue* queue, Task_info* dest_task_info)
{
    rassert(queue != NULL);
    rassert(dest_task_info != NULL);

    Mutex_lock(&queue->lock);

    const bool is_empty = (queue->fetch_index == queue->add_index);
    if (is_empty)
    {
        Mutex_unlock(&queue->lock);
        return false;
    }

    *dest_task_info = queue->tasks[queue->fetch_index];
    if (dest_task_info->state != TASK_INFO_END_QUEUE)
    {
        Task_info_init(&queue->tasks[queue->fetch_index]);
        queue->fetch_index = (queue->fetch_index + 1) % QUEUE_SIZE;
    }

    Mutex_unlock(&queue->lock);

    return true;
}


static void Task_queue_reset(Task_queue* queue)
{
    rassert(queue != NULL);

    queue->add_index = 0;
    queue->fetch_index = 0;

    for (int i = 0; i < QUEUE_SIZE; ++i)
        Task_info_init(&queue->tasks[i]);

    return;
}


static void Task_queue_deinit(Task_queue* queue)
{
    rassert(queue != NULL);

#ifdef ENABLE_THREADS
    Mutex_deinit(&queue->lock);
#endif

    return;
}


struct Background_loader
{
    int thread_count;
    Task_worker workers[KQT_THREADS_MAX];

    int active_task_count;

    Error first_error;

    Condition signal;
    Loader_state state;

    Task_queue work_queue;
    Task_queue cleanup_queue;
};


Background_loader* new_Background_loader(void)
{
    Background_loader* loader = memory_alloc_item(Background_loader);
    if (loader == NULL)
        return NULL;

    loader->thread_count = 0;

    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        Task_worker_init(&loader->workers[i], loader);

    loader->active_task_count = 0;

    loader->first_error = *ERROR_AUTO;

    loader->signal = *CONDITION_AUTO;
    loader->state = LOADER_STATE_INIT;

#ifdef ENABLE_THREADS
    Condition_init(&loader->signal);
#endif

    Task_queue_init(&loader->work_queue);
    Task_queue_init(&loader->cleanup_queue);

    return loader;
}


void Background_loader_set_thread_count(Background_loader* loader, int count)
{
    rassert(loader != NULL);
    rassert(count >= 0);
    rassert(count <= KQT_THREADS_MAX);

#ifdef ENABLE_THREADS
    loader->thread_count = count;
#else
    loader->thread_count = 0;
#endif

    return;
}


int Background_loader_get_thread_count(const Background_loader* loader)
{
    rassert(loader != NULL);
    return loader->thread_count;
}


static void Background_loader_run_cleanups(Background_loader* loader)
{
    rassert(loader != NULL);

#ifdef ENABLE_THREADS
    Task_info* task_info = TASK_INFO_AUTO;
    while (Task_queue_fetch(&loader->cleanup_queue, task_info))
    {
        rassert(task_info->task.cleanup != NULL);
        task_info->task.cleanup(&task_info->error, task_info->task.user_data);

        if (Error_is_set(&task_info->error) && !Error_is_set(&loader->first_error))
            Error_copy(&loader->first_error, &task_info->error);

        --loader->active_task_count;
    }
#endif // ENABLE_THREADS

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

    Background_loader_run_cleanups(loader);
    if (loader->active_task_count + 2 >= QUEUE_SIZE)
        return false;

    // Add new task info
    Task_info* task_info = TASK_INFO_AUTO;
    task_info->task = *task;
    task_info->state = TASK_INFO_READY_TO_START;
    task_info->error = *ERROR_AUTO;

    if (!Task_queue_add(&loader->work_queue, task_info))
        return false;

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
            Task_queue_undo_add(&loader->work_queue);
            return false;
        }
    }

    // Signal threads that there is more work to be done
    Mutex* signal_mutex = Condition_get_mutex(&loader->signal);
    Mutex_lock(signal_mutex);
    loader->state = LOADER_STATE_IN_PROGRESS;
    Condition_broadcast(&loader->signal);
    Mutex_unlock(signal_mutex);

    ++loader->active_task_count;

    return true;
}


static bool Background_loader_fetch_task_info(
        Background_loader* loader, Task_info* dest_task_info)
{
    rassert(loader != NULL);

    bool task_found = false;
    bool all_done = false;

    while (!task_found && !all_done)
    {
        if (Task_queue_fetch(&loader->work_queue, dest_task_info))
        {
            if (dest_task_info->state == TASK_INFO_READY_TO_START)
            {
                dest_task_info->state = TASK_INFO_IN_PROGRESS;
                task_found = true;
            }
            else
            {
                rassert(dest_task_info->state == TASK_INFO_END_QUEUE);
                all_done = true;
            }
        }
        else
        {
            Mutex* signal_mutex = Condition_get_mutex(&loader->signal);
            Mutex_lock(signal_mutex);

            if (loader->state != LOADER_STATE_WAITING)
                Condition_wait(&loader->signal);

            Mutex_unlock(signal_mutex);
        }
    }

    return task_found;
}


static void Background_loader_add_cleanup_task_info(
        Background_loader* loader, Task_info* task_info)
{
    rassert(loader != NULL);
    rassert(task_info != NULL);

    bool success = Task_queue_add(&loader->cleanup_queue, task_info);
    rassert(success);

    return;
}


void Background_loader_wait_idle(Background_loader* loader)
{
    rassert(loader != NULL);

#ifdef ENABLE_THREADS
    // Let all workers know that no more tasks are coming
    Task_info* task_info = TASK_INFO_AUTO;
    task_info->state = TASK_INFO_END_QUEUE;
    Task_queue_add(&loader->work_queue, task_info);

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

    rassert(!(loader->active_task_count > 0));
    rassert(!(loader->active_task_count < 0));

    Task_queue_reset(&loader->work_queue);
    Task_queue_reset(&loader->cleanup_queue);

    return;
}


void del_Background_loader(Background_loader* loader)
{
    if (loader == NULL)
        return;

    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        Task_worker_deinit(&loader->workers[i]);

    Task_queue_deinit(&loader->work_queue);
    Task_queue_deinit(&loader->cleanup_queue);

    memory_free(loader);

    return;
}


