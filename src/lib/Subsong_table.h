

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_SUBSONG_TABLE_H
#define K_SUBSONG_TABLE_H


#include <stdint.h>
#include <stdbool.h>

#include <kunquat/limits.h>
#include <File_base.h>
#include <File_tree.h>
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
 * Reads a Subsong table from a File tree.
 *
 * \param table   The Subsong table -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Subsong_table_read(Subsong_table* table, File_tree* tree, Read_state* state);


/**
 * Sets a Subsong in the Subsong table.
 *
 * \param table     The Subsong table -- must not be \c NULL.
 * \param index     The target index -- must be >= \c 0 and
 *                  < \c KQT_SUBSONGS_MAX.
 * \param subsong   The Subsong -- must not be \c NULL.
 *
 * \return   The actual index where the Subsong was stored, or \c -1 if
 *           memory allocation failed.
 */
int16_t Subsong_table_set(Subsong_table* table, uint16_t index, Subsong* subsong);


/**
 * Gets a Subsong from the Subsong table.
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


