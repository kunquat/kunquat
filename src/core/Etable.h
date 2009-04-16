

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


#ifndef K_ETABLE_H
#define K_ETABLE_H


#include <stdbool.h>


/**
 * This is a simple table object. It can store any type of an object as long
 * as the user provides a destructor function for the type.
 */
typedef struct Etable Etable;


/**
 * Creates a new Etable.
 *
 * \param size      The table size -- must be > \c 0.
 * \param destroy   The destructor for the elements -- must not be \c NULL.
 *
 * \return   The new Etable if successful, or \c NULL if memory allocation
 *           failed.
 */
Etable* new_Etable(int size, void (*destroy)(void*));


/**
 * Inserts a new element into the Etable.
 *
 * If the target index already contains an element, it will be deleted.
 *
 * \param table   The Etable -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 * \param el      The element to be inserted -- must not be \c NULL or
 *                an element already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Etable_set(Etable* table, int index, void* el);


/**
 * Gets an element from the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The element if found, otherwise \c NULL.
 */
void* Etable_get(Etable* table, int index);


/**
 * Removes an element from the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void Etable_remove(Etable* table, int index);


/**
 * Clears the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 */
void Etable_clear(Etable* table);


/**
 * Destroys an existing Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 */
void del_Etable(Etable* table);


#endif // K_ETABLE_H


