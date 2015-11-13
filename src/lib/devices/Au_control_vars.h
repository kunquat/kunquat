

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_AU_CONTROL_VARS_H
#define K_AU_CONTROL_VARS_H


#include <stdlib.h>

#include <kunquat/limits.h>
#include <string/Streader.h>
#include <Value.h>


/**
 * An abstraction layer for controlling internal device parameters during playback.
 */
typedef struct Au_control_vars Au_control_vars;


typedef enum
{
    TARGET_DEV_NONE,
    TARGET_DEV_AU,
    TARGET_DEV_PROC,
} Target_dev_type;


typedef struct Bind_entry Au_control_bind_entry;


typedef struct Au_control_binding_iter
{
    const Au_control_bind_entry* iter;
    int iter_mode;
    Value src_value;
    union
    {
        struct
        {
            double src_range_norm;
        } set_float_type;

        struct
        {
            double osc_range_norm;
        } osc_float_type;
    } ext;

    Target_dev_type target_dev_type;
    int target_dev_index;
    const char* target_var_name;
    Value target_value;
} Au_control_binding_iter;


#define AU_CONTROL_BINDING_ITER_AUTO (&(Au_control_binding_iter){ \
        .iter = NULL, .iter_mode = -1, .target_dev_type = TARGET_DEV_NONE })


/**
 * Start iterating over bound control variable targets.
 *
 * This initialises the iterator to be suitable for retrieving bound target
 * devices only; no mapped target values are calculated.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The name of the variable -- must not be \c NULL.
 *
 * \return   \a true if at least one result is found, otherwise \c false.
 */
bool Au_control_binding_iter_init(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name);


/**
 * Start iterating over results of setting floating-point control variable bindings.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The name of the variable -- must not be \c NULL.
 * \param value      The source value -- must be finite.
 *
 * \return   \c true if at least one result is found, otherwise \c false.
 */
bool Au_control_binding_iter_init_set_float(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name,
        double value);


/**
 * Start iterating over floating-point control variable oscillation depth bindings.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The name of the variable -- must not be \c NULL.
 * \param value      The source oscillation depth -- must be finite.
 *
 * \return   \c true if at least one result is found, otherwise \c false.
 */
bool Au_control_binding_iter_init_osc_depth_float(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name,
        double depth);


/**
 * Get the next result of control variable bindings.
 *
 * \param iter   The iterator -- must not be \c NULL.
 *
 * \return   \c true if the iterator is updated with new result, or \c false if
 *           there are no more results.
 */
bool Au_control_binding_iter_get_next_entry(Au_control_binding_iter* iter);


/**
 * Create new Audio unit control variables.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Audio unit control variables if successful, otherwise \c NULL.
 */
Au_control_vars* new_Au_control_vars(Streader* sr);


/**
 * Destroy existing Audio unit control variables.
 *
 * \param acv   The Audio unit control variables, or \c NULL.
 */
void del_Au_control_vars(Au_control_vars* aucv);


#endif // K_AU_CONTROL_VARS_H


