

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


#ifndef KQT_PAT_TABLE_H
#define KQT_PAT_TABLE_H


#include <containers/Etable.h>
#include <init/sheet/Pattern.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Pat_table Pat_table;


/**
 * Create a new Pattern table.
 *
 * \return   The new Pattern table if successful, or \c NULL if memory
 *           allocation failed.
 */
Pat_table* new_Pat_table(int size);


/**
 * Set the pattern for the specified Pattern table position.
 *
 * \param table   The Pattern table -- must not be \c NULL.
 * \param index   The pattern index -- must be >= \c 0 and
 *                less than the table size.
 * \param pat     The Pattern -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pat_table_set(Pat_table* table, int index, Pattern* pat);


/**
 * Set existent status of a Pattern.
 *
 * \param table      The Pattern table -- must not be \c NULL.
 * \param index      The target index -- must be >= \c 0 and less than the
 *                   table size.
 * \param existent   The new existent status.
 */
void Pat_table_set_existent(Pat_table* table, int index, bool existent);


/**
 * Get existent status of a Pattern.
 *
 * \param table   The Pattern table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The existent status.
 */
bool Pat_table_get_existent(const Pat_table* table, int index);


/**
 * Get the pattern from the specified Pattern table position.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 * \param index     The pattern index -- must be >= \c 0 and
 *                  less than the table size.
 *
 * \return   The Pattern if one exists, otherwise \c NULL.
 */
Pattern* Pat_table_get(Pat_table* table, int index);


/**
 * Remove a Pattern from the Pattern table.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 * \param index     The pattern index -- must be >= \c 0 and
 *                  less than the table size.
 */
void Pat_table_remove(Pat_table* table, int index);


/**
 * Clear the Pattern table.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 */
void Pat_table_clear(Pat_table* table);


/**
 * Destroy an existing Pattern table.
 *
 * \param table   The Pattern table, or \c NULL.
 */
void del_Pat_table(Pat_table* table);


#endif // KQT_PAT_TABLE_H


