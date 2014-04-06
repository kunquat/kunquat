

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_TABLE_H
#define K_DSP_TABLE_H


#include <stdbool.h>

#include <devices/DSP.h>


/**
 * This is the storage object for DSPs and their configurations.
 */
typedef struct DSP_table DSP_table;


/**
 * Create a new DSP table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new DSP table if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP_table* new_DSP_table(int size);


/**
 * Set existent status of a DSP.
 *
 * \param table      The DSP table -- must not be \c NULL.
 * \param index      The target index -- must be >= \c 0 and less than the
 *                   table size.
 * \param existent   The new existent status.
 */
void DSP_table_set_existent(DSP_table* table, int index, bool existent);


/**
 * Insert a new DSP into the DSP table.
 *
 * If the target index already contains a DSP, it will be deleted.
 *
 * \param table   The DSP table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 * \param dsp     The DSP to be inserted -- must not be \c NULL or
 *                a DSP already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool DSP_table_set_dsp(DSP_table* table, int index, DSP* dsp);


/**
 * Get a DSP from the DSP table.
 *
 * \param table   The DSP table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The DSP if found, otherwise \c NULL.
 */
DSP* DSP_table_get_dsp(const DSP_table* table, int index);


/**
 * Remove a DSP from the DSP table.
 *
 * \param table   The DSP table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void DSP_table_remove_dsp(DSP_table* table, int index);


/**
 * Clear the DSP table.
 *
 * \param table   The DSP table -- must not be \c NULL.
 */
void DSP_table_clear(DSP_table* table);


/**
 * Destroy an existing DSP table.
 *
 * \param table   The DSP table, or \c NULL.
 */
void del_DSP_table(DSP_table* table);


#endif // K_DSP_TABLE_H


