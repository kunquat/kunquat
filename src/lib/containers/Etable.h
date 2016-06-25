

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


#ifndef KQT_ETABLE_H
#define KQT_ETABLE_H


#include <stdbool.h>
#include <stdlib.h>


/**
 * This is a simple table object. It can store any type of an object as long
 * as the user provides a destructor function for the type.
 */
typedef struct Etable Etable;


/**
 * Create a new Etable.
 *
 * \param size      The table size -- must be > \c 0.
 * \param destroy   The destructor for the elements -- must not be \c NULL.
 *
 * \return   The new Etable if successful, or \c NULL if memory allocation
 *           failed.
 */
Etable* new_Etable(int size, void (*destroy)(void*));


/**
 * Insert a new element into the Etable.
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
 * Get an element from the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The element if found, otherwise \c NULL.
 */
void* Etable_get(Etable* table, int index);


/**
 * Remove an element from the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void Etable_remove(Etable* table, int index);


/**
 * Clear the Etable.
 *
 * \param table   The Etable -- must not be \c NULL.
 */
void Etable_clear(Etable* table);


/**
 * Destroy an existing Etable.
 *
 * \param table   The Etable, or \c NULL.
 */
void del_Etable(Etable* table);


#endif // KQT_ETABLE_H


