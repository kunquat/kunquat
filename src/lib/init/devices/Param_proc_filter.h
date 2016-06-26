

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


#ifndef KQT_PARAM_PROC_FILTER_H
#define KQT_PARAM_PROC_FILTER_H


#include <decl.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Create a new Parameter processor filter.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Parameter processor filter if successful, otherwise \c NULL.
 */
Param_proc_filter* new_Param_proc_filter(Streader* sr);


/**
 * Get processor filtering information from the Parameter processor filter.
 *
 * \param pf           The Parameter processor filter -- must not be \c NULL.
 * \param proc_index   The processor index -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 *
 * \return   \c true if processor \a proc_index is allowed to activate,
 *           otherwise \c false.
 */
bool Param_proc_filter_is_proc_allowed(const Param_proc_filter* pf, int proc_index);


/**
 * Destroy an existing Processor filter.
 *
 * \param pf   The Processor filter, or \c NULL.
 */
void del_Param_proc_filter(Param_proc_filter* pf);


#endif // KQT_PARAM_PROC_FILTER_H


