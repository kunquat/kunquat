

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


#ifndef K_SUBSONG_H
#define K_SUBSONG_H


#include <stdint.h>

#include <File_base.h>
#include <File_tree.h>


#define ORDER_NONE (-1)


typedef struct Subsong
{
    double tempo;      ///< Initial tempo.
    double global_vol; ///< Initial global volume.
    int notes;         ///< Index of the initial Note table.
    int res;
    int16_t* pats;     ///< Pattern numbers.
} Subsong;


/**
 * Creates a new Subsong.
 *
 * \return   The new Subsong if successful, or \c NULL if memory allocation
 *           failed.
 */
Subsong* new_Subsong(void);


/**
 * Reads a Subsong from a File tree.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Subsong_read(Subsong* ss, File_tree* tree, Read_state* state);


/**
 * Sets the pattern for the specified Subsong position.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and < \c ORDERS_MAX.
 * \param pat     The pattern number -- must be >= \c 0 or ORDER_NONE.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Subsong_set(Subsong* ss, int index, int16_t pat);


/**
 * Gets the pattern from the specified Subsong position.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and < \c ORDERS_MAX.
 *
 * \return   The pattern number if one exists, otherwise ORDER_NONE.
 */
int16_t Subsong_get(Subsong* ss, int index);


/**
 * Sets the initial tempo of the Subsong.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param tempo   The tempo -- must be finite and positive.
 */
void Subsong_set_tempo(Subsong* ss, double tempo);


/**
 * Gets the initial tempo of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The tempo.
 */
double Subsong_get_tempo(Subsong* ss);


/**
 * Sets the initial global volume of the Subsong.
 *
 * \param ss    The Subsong -- must not be \c NULL.
 * \param vol   The global volume -- must be finite or \c -INFINITY.
 */
void Subsong_set_global_vol(Subsong* ss, double vol);


/**
 * Gets the initial global volume of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The global volume.
 */
double Subsong_get_global_vol(Subsong* ss);


/**
 * Sets the initial default Note table of the Subsong.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The Note table index -- must be >= \c 0 and
 *                < \c NOTE_TABLES_MAX.
 */
void Subsong_set_notes(Subsong* ss, int index);


/**
 * Gets the initial default Note table of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The Note table index.
 */
int Subsong_get_notes(Subsong* ss);


/**
 * Clears the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 */
// void Subsong_clear(Subsong* ss);


/**
 * Destroys an existing Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 */
void del_Subsong(Subsong* ss);


#endif // K_SUBSONG_H


