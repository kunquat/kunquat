

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


#ifndef KQT_MIXED_SIGNAL_PLAN_BUILD_H
#define KQT_MIXED_SIGNAL_PLAN_BUILD_H


#include <decl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Mixed_signal_task_info Mixed_signal_task_info;


/**
 * Create a new Mixed signal task information context.
 *
 * \param device_id     The Device ID.
 * \param level_index   The level index -- must be >= \c 0.
 *
 * \return   The new Mixed signal task information context if successful, or
 *           \c NULL if memory allocation failed.
 */
Mixed_signal_task_info* new_Mixed_signal_task_info(uint32_t device_id, int level_index);


#if 0
/**
 * Add new signal input information to the Mixed signal task information context.
 *
 * \param task_info   The context -- must not be \c NULL.
 * \param recv_port   The input port of the receiving device -- must be valid.
 * \param sender_id   The sender device ID.
 * \param send_port   The output port of the sender device -- must be valid.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Mixed_signal_task_info_add_input(
        Mixed_signal_task_info* task_info,
        int recv_port,
        uint32_t sender_id,
        int send_port);
#endif


/**
 * Destroy an existing Mixed signal task information context.
 *
 * \param task_info   The context, or \c NULL.
 */
void del_Mixed_signal_task_info(Mixed_signal_task_info* task_info);


/**
 * Increase the level of an existing Mixed signal task information context.
 *
 * \param plan              The Mixed signal plan -- must not be \c NULL.
 * \param device_id         The Device ID associated with the context.
 * \param new_level_index   The new level index -- must be >= \c 0.
 *
 * \return   \c true if there is a context associated with \a device_id,
 *           otherwise \c false.
 */
bool Mixed_signal_plan_try_increase_level(
        Mixed_signal_plan* plan, uint32_t device_id, int new_level_index);


/**
 * Add a task information context to the Mixed signal plan.
 *
 * It is an error to call this after calling Mixed_signal_plan_finalise for the
 * given Mixed signal plan.
 *
 * \param plan        The Mixed signal plan -- must not be \c NULL.
 * \param task_info   Context information for task execution -- must not be
 *                    \c NULL. \a plan will assume ownership of \a task_info
 *                    if and only if this function call succeeds.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Mixed_signal_plan_add_task(
        Mixed_signal_plan* plan, Mixed_signal_task_info* task_info);


/**
 * Finalise construction of the Mixed signal plan.
 *
 * This must be called exactly once after all tasks have been added.
 *
 * \param plan   The Mixed signal plan -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Mixed_signal_plan_finalise(Mixed_signal_plan* plan);


#endif // KQT_MIXED_SIGNAL_PLAN_BUILD_H


