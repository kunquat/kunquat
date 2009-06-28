

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


#ifndef K_INS_TABLE_H
#define K_INS_TABLE_H


#include <stdbool.h>

#include <Instrument.h>


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
 * Reads an Instrument table from a File tree.
 *
 * \param table   The Pattern table -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 * \param song    The Song -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Ins_table_read(Ins_table* table, File_tree* tree, Read_state* state,
                    frame_t** bufs,
                    frame_t** voice_bufs,
                    int buf_count,
                    uint32_t buf_len,
                    Note_table** note_tables,
                    Note_table** default_notes,
                    uint8_t events);


/**
 * Inserts a new Instrument into the Instrument table.
 *
 * If the target index already contains an Instrument, it will be deleted.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 * \param index   The target index -- must be > \c 0 and not greater than
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
 * \param index   The target index -- must be > \c 0 and not greater than
 *                the table size.
 *
 * \return   The Instrument if found, otherwise \c NULL.
 */
Instrument* Ins_table_get(Ins_table* table, int index);


/**
 * Removes an Instrument from the Instrument table.
 *
 * \param table   The Instrument table -- must not be \c NULL.
 * \param index   The target index -- must be > \c 0 and not greater than
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
 * \param table   The Instrument table -- must not be \c NULL.
 */
void del_Ins_table(Ins_table* table);


#endif // K_INS_TABLE_H


