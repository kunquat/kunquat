

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


#ifndef KQT_PROC_TABLE_H
#define KQT_PROC_TABLE_H


#include <init/devices/Processor.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * This is the storage object for Processors and their configurations.
 */
typedef struct Proc_table Proc_table;


/**
 * Create a new Processor table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Processor table if successful, or \c NULL if memory
 *           allocation failed.
 */
Proc_table* new_Proc_table(int size);


/**
 * Set existent status of a Processor.
 *
 * \param table      The Processor table -- must not be \c NULL.
 * \param index      The target index -- must be >= \c 0 and less than the
 *                   table size.
 * \param existent   The new existent status.
 */
void Proc_table_set_existent(Proc_table* table, int index, bool existent);


/**
 * Insert a new Processor into the Processor table.
 *
 * If the target index already contains a Processor, it will be deleted.
 *
 * \param table   The Processor table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 * \param proc    The Processor to be inserted -- must not be \c NULL or a
 *                Processor already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Proc_table_set_proc(Proc_table* table, int index, Processor* proc);


/**
 * Get a Processor from the Processor table.
 *
 * \param table   The Processor table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Processor if found, otherwise \c NULL.
 */
const Processor* Proc_table_get_proc(const Proc_table* table, int index);


/**
 * Get a mutable Processor from the Processor table.
 *
 * \param table   The Processor table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Processor if found, otherwise \c NULL.
 */
Processor* Proc_table_get_proc_mut(Proc_table* table, int index);


/**
 * Remove a Processor from the Processor table.
 *
 * \param table   The Processor table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 */
void Proc_table_remove_proc(Proc_table* table, int index);


/**
 * Clear the Processor table.
 *
 * \param table   The Processor table -- must not be \c NULL.
 */
void Proc_table_clear(Proc_table* table);


/**
 * Destroy an existing Processor table.
 *
 * \param table   The Processor table, or \c NULL.
 */
void del_Proc_table(Proc_table* table);


#endif // KQT_PROC_TABLE_H


