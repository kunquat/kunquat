

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_MIXED_SIGNAL_PLAN_H
#define KQT_MIXED_SIGNAL_PLAN_H


#include <decl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Create a new Mixed signal plan.
 *
 * \param dstates   The Device states -- must not be \c NULL.
 * \param conns     The Connections -- must not be \c NULL.
 *
 * \return   The new Mixed signal plan if successful, or \c NULL if memory
 *           allocation failed.
 */
Mixed_signal_plan* new_Mixed_signal_plan(
        Device_states* dstates, const Connections* conns);


/**
 * Get the number of levels in the Mixed signal plan.
 *
 * \param plan   The Mixed signal plan -- must not be \c NULL.
 *
 * \return   The number of levels in \a plan.
 */
int Mixed_signal_plan_get_level_count(const Mixed_signal_plan* plan);


/**
 * Reset the Mixed signal plan.
 *
 * \param plan   The Mixed signal plan -- must not be \c NULL.
 */
void Mixed_signal_plan_reset(Mixed_signal_plan* plan);


#ifdef ENABLE_THREADS
/**
 * Execute a task in the Mixed signal plan.
 *
 * \param plan          The Mixed signal plan -- must not be \c NULL.
 * \param level_index   The level index of execution -- must be >= \c 0 and
 *                      < Mixed_signal_plan_get_level_count(\a plan). Additionally,
 *                      the first level to be executed must be exactly
 *                      Mixed_signal_plan_get_level_count(\a plan) - 1, and
 *                      \a level must decrease by \c 1 after each time this function
 *                      returns \c false.
 * \param wbs           The Work buffers -- must not be \c NULL.
 * \param buf_start     The start index of buffer areas to be processed
 *                      -- must be less than the buffer size.
 * \param buf_stop      The stop index of buffer areas to be processed
 *                      -- must not be greater than the buffer size.
 * \param tempo         The current tempo -- must be > \c 0.
 *
 * \return   \c true if there may be more tasks left to be executed in
 *           \a level, otherwise \c false.
 */
bool Mixed_signal_plan_execute_next_task(
        Mixed_signal_plan* plan,
        int level_index,
        Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);
#endif


/**
 * Execute all tasks in the Mixed signal plan using a single thread.
 *
 * \param plan        The Mixed signal plan -- must not be \c NULL.
 * \param wbs         The Work buffers -- must not be \c NULL.
 * \param buf_start   The start index of buffer areas to be processed
 *                    -- must be less than the buffer size.
 * \param buf_stop    The stop index of buffer areas to be processed
 *                    -- must not be greater than the buffer size.
 * \param tempo       The current tempo -- must be > \c 0.
 */
void Mixed_signal_plan_execute_all_tasks(
        Mixed_signal_plan* plan,
        Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


/**
 * Destroy an existing Mixed signal plan.
 *
 * \param plan   The Mixed signal plan, or \c NULL.
 */
void del_Mixed_signal_plan(Mixed_signal_plan* plan);


#endif // KQT_MIXED_SIGNAL_PLAN_H


