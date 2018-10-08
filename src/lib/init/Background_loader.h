

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


#ifndef KQT_BACKGROUND_LOADER_H
#define KQT_BACKGROUND_LOADER_H


#include <decl.h>
#include <Error.h>

#include <stdbool.h>
#include <stdlib.h>


typedef void Background_loader_callback(Error* error, void* user_data);


typedef struct Background_loader_task
{
    Background_loader_callback* callback;
    void* user_data;
    bool is_finished;
    Error* error;
} Background_loader_task;


#define MAKE_BACKGROUND_LOADER_TASK(cb, ud) (&(Background_loader_task){ \
        .callback = (cb), .user_data = (ud), .is_finished = false, .error = NULL })


/**
 * Create a new Background loader.
 *
 * \return   The new Background loader, or \c NULL if memory allocation failed.
 */
Background_loader* new_Background_loader(void);


/**
 * Set the number of background threads used by the Background loader.
 *
 * \param loader   The Background loader -- must not be \c NULL.
 * \param count    The number of threads -- must be >= \c 0. If \c 0, all calls
 *                 of \a Background_loader_execute_task will block until finished.
 * \param error    Destination for error information -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Background_loader_set_thread_count(
        Background_loader* loader, int count, Error* error);


/**
 * Get the number of background threads used by the Background loader.
 *
 * \param loader   The Background loader -- must not be \c NULL.
 *
 * \return   The number of background threads.
 */
int Background_loader_get_thread_count(const Background_loader* loader);


/**
 * Execute a task in the Background loader.
 *
 * \param loader   The Background loader -- must not be \c NULL.
 * \param task     The Background loader task -- must not be \c NULL.
 *
 * \return   \c true if \a task was successfully put into the task queue,
 *           otherwise \c false.
 */
bool Background_loader_add_task(Background_loader* loader, Background_loader_task* task);


/**
 * Wait until all tasks currently in the Background loader are finished.
 *
 * \param loader   The Background loader -- must not be \c NULL.
 */
void Background_loader_wait_idle(Background_loader* loader);


/**
 * Get the first error set in the Background loader.
 *
 * \param loader   The Background loader -- must not be \c NULL.
 *
 * \return   The first Error that occurred, or \c NULL if everything has completed
 *           successfully.
 */
Error* Background_loader_get_first_error(const Background_loader* loader);


/**
 * Reset the Background loader.
 *
 * \param loader   The Background loader -- must not be \c NULL and must not have
 *                 tasks in execution.
 */
void Background_loader_reset(Background_loader* loader);


/**
 * Destroy an existing Background loader.
 *
 * \param loader   The Background loader, or \c NULL.
 */
void del_Background_loader(Background_loader* loader);


#endif // KQT_BACKGROUND_LOADER_H


