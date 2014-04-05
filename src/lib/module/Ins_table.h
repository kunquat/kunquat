

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


#ifndef K_INS_TABLE_H
#define K_INS_TABLE_H


#include <stdbool.h>

#include <devices/Instrument.h>


/**
 * This is the storage object for Instruments.
 */
typedef struct Ins_table Ins_table;


/**
 * Creates a new Instrument table.
 *
 * \param size   The table size -- must be > \c 0.
 *
 * \return   The new Instrument table if successful, or \c NULL if memory
 *           allocation failed.
 */
Ins_table* new_Ins_table(int size);


/**
 * Inserts a new Instrument into the Instrument table.
 *
 * If the target index already contains an Instrument, it will be deleted.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 * \param ins     The Instrument to be inserted -- must not be \c NULL or
 *                an Instrument already stored in the table.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Ins_table_set(Ins_table* table, int index, Instrument* ins);


/**
 * Gets an Instrument from the Instrument table.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 *
 * \return   The Instrument if found, otherwise \c NULL.
 */
Instrument* Ins_table_get(Ins_table* table, int index);


/**
 * Removes an Instrument from the Instrument table.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and less than
 *                the table size.
 */
void Ins_table_remove(Ins_table* table, int index);


/**
 * Clears the Instrument table.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 */
void Ins_table_clear(Ins_table* table);


/**
 * Destroys an existing Instrument table.
 *
 * \param table   The Instrument table, or \c NULL.
 */
void del_Ins_table(Ins_table* table);


#endif // K_INS_TABLE_H


