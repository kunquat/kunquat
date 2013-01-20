

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <Generator.h>
#include <Gen_conf.h>


/**
 * This is the storage object for Generators and their configurations.
 */
typedef struct Gen_table Gen_table;


/**
 * Creates a new Generator table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Generator table if successful, or \c NULL if memory
 *           allocation failed.
 */
Gen_table* new_Gen_table(int size);


/**
 * Sets existent status of a Generator.
 *
 * \param table      The Generator table -- must not be \c NULL.
 * \param index      The target index -- must be >= \c 0 and less than the
 *                   table size.
 * \param existent   The new existent status.
 */
void Gen_table_set_existent(Gen_table* table, int index, bool existent);


/**
 * Inserts a new Generator configuration into the Generator table.
 *
 * If the target index already contains a Generator configuration, it will be
 * deleted. Also, the corresponding Generator, if one exists, will be updated.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 * \param conf    The Generator configuration -- must not be \c NULL or a
 *                Generator configuration already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Gen_table_set_conf(Gen_table* table, int index, Gen_conf* conf);


/**
 * Gets a Generator configuration from the Generator table.
 *
 * NOTE: On success, this function always returns a valid Generator
 * configuration. Failure to do so indicates memory allocation error.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Generator configuration, or \c NULL if memory allocation
 *           failed.
 */
Gen_conf* Gen_table_get_conf(Gen_table* table, int index);


/**
 * Inserts a new Generator into the Generator table.
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
 * Gets a Generator from the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 *
 * \return   The Generator if found, otherwise \c NULL.
 */
Generator* Gen_table_get_gen(Gen_table* table, int index);


/**
 * Removes a Generator from the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than the
 *                table size.
 */
void Gen_table_remove_gen(Gen_table* table, int index);


/**
 * Clears the Generator table.
 *
 * \param table   The Generator table -- must not be \c NULL.
 */
void Gen_table_clear(Gen_table* table);


/**
 * Destroys an existing Generator table.
 *
 * \param table   The Generator table, or \c NULL.
 */
void del_Gen_table(Gen_table* table);


#endif // K_GEN_TABLE_H


