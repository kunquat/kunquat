

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SUBSONG_TABLE_H
#define K_SUBSONG_TABLE_H


#include <stdint.h>
#include <stdbool.h>

#include <kunquat/limits.h>
#include <File_base.h>
#include <Subsong.h>


/**
 * Subsong table contains the Subsongs.
 */
typedef struct Subsong_table Subsong_table;


/**
 * Creates a new Subsong table.
 *
 * \return   The new Subsong table if successful, or \c NULL if memory
 *           allocation failed.
 */
Subsong_table* new_Subsong_table(void);


/**
 * Sets a Subsong in the Subsong table.
 *
 * \param table     The Subsong table -- must not be \c NULL.
 * \param index     The target index -- must be >= \c 0 and
 *                  < \c KQT_SUBSONGS_MAX.
 * \param subsong   The Subsong -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Subsong_table_set(Subsong_table* table, uint16_t index, Subsong* subsong);


/**
 * Gets a Subsong from the Subsong table.
 *
 * Note: Subsongs after an empty index are considered hidden and are not
 * returned.
 *
 * \param table   The Subsong table -- must not be \c NULL.
 * \param index   The subsong number -- must be >= \c 0 and
 *                < \c KQT_SUBSONGS_MAX.
 *
 * \return   The Subsong if one exists, otherwise \c NULL.
 */
Subsong* Subsong_table_get(Subsong_table* table, uint16_t subsong);


/**
 * Tells whether a subsong is empty.
 *
 * \param table     The Subsong table -- must not be \c NULL.
 * \param subsong   The subsong number -- must be >= \c 0 and
 *                  < \c KQT_SUBSONGS_MAX.
 *
 * \return   \c true if and only if \a subsong is empty.
 */
bool Subsong_table_is_empty(Subsong_table* table, uint16_t subsong);


/**
 * Destroys an existing Subsong table.
 *
 * \param table   The Subsong table -- must not be \c NULL.
 */
void del_Subsong_table(Subsong_table* table);


#endif // K_SUBSONG_TABLE_H


