

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


#ifndef K_GEN_TABLE_H
#define K_GEN_TABLE_H


#include <stdbool.h>

#include <devices/Generator.h>


/**
 * This is the storage object for Generators and their configurations.
 */
typedef struct Gen_table Gen_table;


/**
 * Create a new Generator table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Generator table if successful, or \c NULL if memory
 *           allocation failed.
 */
Gen_table* new_Gen_table(int size);


/**
 * Set existent status of a Generator.
 *
 * \param table      The Generator table -- must not be \c NULL.
 * \param index      The target index -- must be >= \c 0 and less than the
 *                   table size.
 * \param existent   The new existent status.
 */
void Gen_table_set_existent(Gen_table* table, int index, bool existent);


/**
 * Insert a new Generator into the Generator table.
 *
 * If the target index already contains a Generator, it will be deleted.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 * \param gen     The Generator to be inserted -- must not be \c NULL or a
 *                Generator already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Gen_table_set_gen(Gen_table* table, int index, Generator* gen);


/**
 * Get a Generator from the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
const Generator* Gen_table_get_gen(const Gen_table* table, int index);


/**
 * Get a mutable Generator from the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
Generator* Gen_table_get_gen_mut(Gen_table* table, int index);


/**
 * Remove a Generator from the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 */
void Gen_table_remove_gen(Gen_table* table, int index);


/**
 * Clear the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 */
void Gen_table_clear(Gen_table* table);


/**
 * Destroy an existing Generator table.
 *
 * \param table   The Generator table, or \c NULL.
 */
void del_Gen_table(Gen_table* table);


#endif // K_GEN_TABLE_H


