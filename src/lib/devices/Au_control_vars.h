

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


#include <containers/AAtree.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <player/Linear_controls.h>
#include <string/Streader.h>
#include <Value.h>

#include <stdlib.h>


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


typedef struct Au_control_var_iter
{
    AAiter iter;
    const char* next_var_name;
    Value_type next_var_type;
} Au_control_var_iter;

#define AU_CONTROL_VAR_ITER_AUTO \
    (&(Au_control_var_iter){ { .tree = NULL }, NULL, VALUE_TYPE_NONE })


typedef struct Bind_entry Au_control_bind_entry;

typedef struct Au_control_binding_iter
{
    // Internal state
    const Au_control_bind_entry* iter;
    int iter_mode;
    Value src_value;
    Random* rand;
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

        struct
        {
            double range_min;
            double range_max;
            Linear_controls src_controls;
        } init_float_type;
    } ext;

    // Data provided for the user
    Target_dev_type target_dev_type;
    int target_dev_index;
    const char* target_var_name;
    Value target_value;
    Linear_controls target_controls;
} Au_control_binding_iter;

#define AU_CONTROL_BINDING_ITER_AUTO (&(Au_control_binding_iter){ \
        .iter = NULL, .iter_mode = -1, .target_dev_type = TARGET_DEV_NONE })


/**
 * Initialise an Audio unit control variable iterator.
 *
 * \param iter   The Audio unit control variable iterator -- must not be \c NULL.
 * \param aucv   The Audio unit control variables -- must not be \c NULL.
 *
 * \return   The parameter \a iter.
 */
Au_control_var_iter* Au_control_var_iter_init(
        Au_control_var_iter* iter, const Au_control_vars* aucv);


/**
 * Get the next Audio unit control variable name from the iterator.
 *
 * \param iter           The Audio unit control variable iterator --
 *                       must not be \c NULL.
 * \param out_var_name   The destination address of the variable name --
 *                       must not be \c NULL. A \c NULL value will be stored if
 *                       the end of iteration has been reached.
 * \param out_var_type   The destination address of the variable type --
 *                       must not be \c NULL.
 */
void Au_control_var_iter_get_next_var_info(
        Au_control_var_iter* iter, const char** out_var_name, Value_type* out_var_type);


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
 * Start iterating over results of setting control variables.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param rand       The Random source -- must not be \c NULL unless the
 *                   control variable is expected to be a sliding float.
 * \param var_name   The name of the variable -- must not be \c NULL.
 * \param value      The source Value -- must not be \c NULL.
 *
 * \return   \c true if at least one bound target is found, otherwise \c false.
 *           NOTE: The target value may have type set to \c VALUE_TYPE_NONE
 *           if expression evaluation fails or the result cannot be converted
 *           to the type specified by the binding.
 */
bool Au_control_binding_iter_init_set_generic(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        Random* rand,
        const char* var_name,
        const Value* value);


/**
 * Start iterating over results of sliding bound floating-point control variables.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The name of the variable -- must not be \c NULL.
 * \param value      The source value -- must be finite.
 *
 * \return   \c true if at least one result is found, otherwise \c false.
 */
bool Au_control_binding_iter_init_slide_float(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name,
        double value);


/**
 * Start iterating over oscillation depths of bound floating-point control variables.
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
 * Start iterating over float controls of bound floating-point control variables.
 *
 * \param iter       The iterator -- must not be \c NULL.
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The name of the variable -- must not be \c NULL.
 * \param controls   The source Linear controls -- must not be \c NULL.
 *
 * \return   \c true if at least one result is found, otherwise \c false.
 */
bool Au_control_binding_iter_init_float_controls(
        Au_control_binding_iter* iter,
        const Au_control_vars* aucv,
        const char* var_name,
        const Linear_controls* controls);


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
 * Get the initial value of a control variable.
 *
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL and must exist
 *                   in \a aucv.
 *
 * \return   The initial Value. This is never \c NULL.
 */
const Value* Au_control_vars_get_init_value(
        const Au_control_vars* aucv, const char* var_name);


/**
 * Tell whether a control variable is of any floating type.
 *
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL and must exist
 *                   in \a aucv.
 *
 * \return   \c true if \a var_name refers to a floating type (with or without
 *           mapped range), otherwise \c false.
 */
bool Au_control_vars_in_entry_any_float(
        const Au_control_vars* aucv, const char* var_name);


/**
 * Tell whether a control variable is of floating slide type.
 *
 * \param aucv       The Audio unit control variables -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL and must exist
 *                   in \a aucv.
 *
 * \return   \c true if \a var_name refers to a floating slide type, otherwise \c false.
 */
bool Au_control_vars_is_entry_float_slide(
        const Au_control_vars* aucv, const char* var_name);


/**
 * Destroy existing Audio unit control variables.
 *
 * \param aucv   The Audio unit control variables, or \c NULL.
 */
void del_Au_control_vars(Au_control_vars* aucv);


#endif // K_AU_CONTROL_VARS_H


