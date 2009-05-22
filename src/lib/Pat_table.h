

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


#ifndef K_PAT_TABLE_H
#define K_PAT_TABLE_H


#include <stdbool.h>

#include <Etable.h>
#include <Pattern.h>


typedef struct Pat_table
{
    int size;
    Etable* pats;
} Pat_table;


/**
 * Creates a new Pattern table.
 *
 * \return   The new Pattern table if successful, or \c NULL if memory
 *           allocation failed.
 */
Pat_table* new_Pat_table(int size);


/**
 * Sets the pattern for the specified Pattern table position.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 * \param index     The pattern index -- must be >= \c 0 and
 *                  less than the table size.
 * \param pat       The Pattern -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pat_table_set(Pat_table* table, int index, Pattern* pat);


/**
 * Gets the pattern from the specified Pattern table position.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 * \param index     The pattern index -- must be >= \c 0 and
 *                  less than the table size.
 *
 * \return   The Pattern if one exists, otherwise \c NULL.
 */
Pattern* Pat_table_get(Pat_table* table, int index);


/**
 * Removes a Pattern from the Pattern table.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 * \param index     The pattern index -- must be >= \c 0 and
 *                  less than the table size.
 */
void Pat_table_remove(Pat_table* table, int index);


/**
 * Clears the Pattern table.
 *
 * \param table     The Pattern table -- must not be \c NULL.
 */
void Pat_table_clear(Pat_table* table);


/**
 * Destroys an existing Pattern table.
 *
 * \param table   The Pattern table -- must not be \c NULL.
 */
void del_Pat_table(Pat_table* table);


#endif // K_PAT_TABLE_H


