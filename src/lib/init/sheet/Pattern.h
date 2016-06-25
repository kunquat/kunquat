

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PATTERN_H
#define KQT_PATTERN_H


#include <containers/AAtree.h>
#include <init/sheet/Column.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <Pat_inst_ref.h>
#include <string/Streader.h>

#include <stdlib.h>


/**
 * This object contains a (typically short) section of music.
 */
typedef struct Pattern Pattern;


#define PATTERN_DEFAULT_LENGTH (Tstamp_set(TSTAMP_AUTO, 16, 0))


/**
 * Create a new Pattern object.
 *
 * The caller shall eventually call del_Pattern() to destroy the Pattern
 * returned.
 *
 * \return   The new Pattern object if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern* new_Pattern(void);


/**
 * Read the length of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 * \param sr    The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Pattern_read_length(Pattern* pat, Streader* sr);


/**
 * Set existent status of a Pattern instance.
 *
 * \param pat        The Pattern -- must not be \c NULL.
 * \param index      The instance index -- must be >= \c 0 and
 *                   < \c KQT_PAT_INSTANCES_MAX.
 * \param existent   The new existence status.
 */
void Pattern_set_inst_existent(Pattern* pat, int index, bool existent);


/**
 * Get existent status of a Pattern instance.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The instance index -- must be >= \c 0 and
 *                < \c KQT_PAT_INSTANCES_MAX.
 *
 * \return   \c true if instance \a index of \a pat exists, otherwise \c false.
 */
bool Pattern_get_inst_existent(const Pattern* pat, int index);


/**
 * Replace a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 * \param col     The Column -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pattern_set_column(Pattern* pat, int index, Column* col);


/**
 * Return a column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 *
 * \return   The column.
 */
Column* Pattern_get_column(const Pattern* pat, int index);


/**
 * Get the length of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 *
 * \return   The length -- must not be freed.
 */
const Tstamp* Pattern_get_length(const Pattern* pat);


/**
 * Destroy an existing Pattern.
 *
 * \param pat   The Pattern, or \c NULL.
 */
void del_Pattern(Pattern* pat);


#endif // KQT_PATTERN_H


