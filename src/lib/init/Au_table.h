

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


#ifndef KQT_AU_TABLE_H
#define KQT_AU_TABLE_H


#include <decl.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * This is the storage object for Audio units.
 */
//typedef struct Au_table Au_table;


/**
 * Create a new Audio unit table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Audio unit table if successful, or \c NULL if memory
 *           allocation failed.
 */
Au_table* new_Au_table(int size);


/**
 * Insert a new Audio unit into the Audio unit table.
 *
 * If the target index already contains an Audio unit, it will be deleted.
 *
 * \param table   The Audio unit table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 * \param au      The Audio unit to be inserted -- must not be \c NULL or
 *                an Audio unit already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Au_table_set(Au_table* table, int index, Audio_unit* au);


/**
 * Get an Audio unit from the Audio unit table.
 *
 * \param table   The Audio unit table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The Audio unit if found, otherwise \c NULL.
 */
Audio_unit* Au_table_get(Au_table* table, int index);


/**
 * Remove an Audio unit from the Audio unit table.
 *
 * \param table   The Audio unit table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void Au_table_remove(Au_table* table, int index);


/**
 * Clear the Audio unit table.
 *
 * \param table   The Audio unit table -- must not be \c NULL.
 */
void Au_table_clear(Au_table* table);


/**
 * Destroy an existing Audio unit table.
 *
 * \param table   The Audio unit table, or \c NULL.
 */
void del_Au_table(Au_table* table);


#endif // KQT_AU_TABLE_H


