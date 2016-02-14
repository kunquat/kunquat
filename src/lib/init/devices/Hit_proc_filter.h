

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


#ifndef K_HIT_PROC_FILTER_H
#define K_HIT_PROC_FILTER_H


#include <decl.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Create a new Hit processor filter.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Hit processor filter if successful, otherwise \c NULL.
 */
Hit_proc_filter* new_Hit_proc_filter(Streader* sr);


/**
 * Get processor filtering information from the Hit processor filter.
 *
 * \param hpf          The Hit processor filter -- must not be \c NULL.
 * \param proc_index   The processor index -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 *
 * \return   \c true if processor \a proc_index is allowed to activate,
 *           otherwise \c false.
 */
bool Hit_proc_filter_is_proc_allowed(const Hit_proc_filter* hpf, int proc_index);


/**
 * Destroy an existing Hit processor filter.
 *
 * \param hpf   The Hit processor filter, or \c NULL.
 */
void del_Hit_proc_filter(Hit_proc_filter* hpf);


#endif // K_HIT_PROC_FILTER_H


