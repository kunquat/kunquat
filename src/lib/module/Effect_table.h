

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EFFECT_TABLE_H
#define K_EFFECT_TABLE_H


#include <stdbool.h>

#include <devices/Effect.h>


/**
 * This is the storage object for Effects.
 */
typedef struct Effect_table Effect_table;


/**
 * Creates a new Effect table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Effect table if successful, or \c NULL if memory
 *           allocation failed.
 */
Effect_table* new_Effect_table(int size);


/**
 * Inserts a new Effect into the Effect table.
 *
 * If the target index already contains an Effect, it will be deleted.
 *
 * \param table   The Effect table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 * \param eff     The Effect to be inserted -- must not be \c NULL or
 *                an Effect already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Effect_table_set(Effect_table* table, int index, Effect* eff);


/**
 * Gets an Effect from the Effect table.
 *
 * \param table   The Effect table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The Effect if found, otherwise \c NULL.
 */
const Effect* Effect_table_get(const Effect_table* table, int index);


/**
 * Gets a mutable Effect from the Effect table.
 *
 * \param table   The Effect table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The Effect if found, otherwise \c NULL.
 */
Effect* Effect_table_get_mut(Effect_table* table, int index);


/**
 * Removes an Effect from the Effect table.
 *
 * \param table   The Effect table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void Effect_table_remove(Effect_table* table, int index);


/**
 * Clears the Effect table.
 *
 * \param table   The Effect table -- must not be \c NULL.
 */
void Effect_table_clear(Effect_table* table);


/**
 * Destroys an existing Effect table.
 *
 * \param table   The Effect table, or \c NULL.
 */
void del_Effect_table(Effect_table* table);


#endif // K_EFFECT_TABLE_H


