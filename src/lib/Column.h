

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


#ifndef K_COLUMN_H
#define K_COLUMN_H


#include <stdbool.h>
#include <stdint.h>

#include <Reltime.h>
#include <Event.h>
#include <AAtree.h>


/**
 * Column_iter is used for retrieving Events from a Column.
 */
typedef struct Column_iter Column_iter;


/**
 * Column is a container for Events in a Pattern. It typically contains a
 * "monophonic" section of music, or global Events.
 */
typedef struct Column Column;


/**
 * Creates a new Column iterator.
 *
 * \param col   The Column associated with the iterator.
 */
Column_iter* new_Column_iter(Column* col);


/**
 * Changes the Column associated with the Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 * \param col    The Column -- must not be \c NULL.
 */
void Column_iter_change_col(Column_iter* iter, Column* col);


/**
 * Gets an Event from the Column.
 *
 * The first Event with position greater than or equal to the given position
 * will be returned.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 * \param pos    The position of the Event -- must not be \c NULL.
 *
 * \return   The Event if one exists, otherwise \c NULL.
 */
Event* Column_iter_get(Column_iter* iter, const Reltime* pos);


/**
 * Gets the Event next to the previous Event retrieved from the Column.
 *
 * If not preceded by a successful call to Column_iter_get(), \c NULL will be
 * returned.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 *
 * \return   The Event if one exists, otherwise \c NULL.
 */
Event* Column_iter_get_next(Column_iter* iter);


/**
 * Destroys an existing Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 */
void del_Column_iter(Column_iter* iter);


/**
 * Creates a new Column.
 *
 * \param length   The length of the column. If this is \c NULL, the length is
 *                 set to INT64_MAX beats.
 *
 * \return   The new Column if successful, or \c NULL if memory allocation
 *           failed.
 */
Column* new_Column(Reltime* len);


/**
 * Inserts a new Event into the Column.
 *
 * If other Events are already located at the target position, the new Event
 * will be placed after these Events.
 *
 * \param col     The Column -- must not be \c NULL.
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Column_ins(Column* col, Event* event);


/**
 * Moves an Event inside a row of the Column.
 *
 * \param col     The Column -- must not be \c NULL.
 * \param event   The Event to be moved -- must not be \c NULL.
 * \param index   The new 0-based index of the Event in the row.
 *
 * \return   \c true if the Column changed, otherwise \c false.
 */
bool Column_move(Column* col, Event* event, unsigned int index);


/**
 * Removes an Event from the Column.
 *
 * \param col     The Column -- must not be \c NULL.
 * \param event   The Event to be removed -- must not be \c NULL.
 *
 * \return   \c true if \a event was found and removed, otherwise \c false.
 */
bool Column_remove(Column* col, Event* event);


/**
 * Removes a row from the Column.
 *
 * \param col   The Column -- must not be \c NULL.
 * \param pos   The position of the row -- must not be \c NULL.
 *
 * \return   \c true if Events were found at \a pos and removed, otherwise
 *           \a false.
 */
bool Column_remove_row(Column* col, Reltime* pos);


/**
 * Removes a block of rows from the Column.
 *
 * \param col     The Column -- must not be \c NULL.
 * \param start   The start of the block -- must not be \c NULL.
 * \param end     The end of the block -- must not be \c NULL. If this is
 *                < \a start, nothing will be removed.
 *
 * \return   \c true if rows were found and removed, otherwise \c false.
 */
bool Column_remove_block(Column* col, Reltime* start, Reltime* end);


/**
 * Shifts Events backward in time.
 *
 * \param col   The Column -- must not be \c NULL.
 * \param pos   The position after which Events shall be shifted -- must not
 *              be \c NULL.
 * \param len   The amount of shifting to be made -- must not be \c NULL.
 *              Events within [pos,pos+len) will be removed.
 *
 * \return   \c true if Events were removed, otherwise \c false.
 */
bool Column_shift_up(Column* col, Reltime* pos, Reltime* len);


/**
 * Shifts Events forward in time.
 *
 * \param col   The Column -- must not be \c NULL.
 * \param pos   The position after which Events shall be shifted -- must not
 *              be \c NULL.
 * \param len   The amount of shifting to be made -- must not be \c NULL.
 *              Events moved past the end of the Column will not be removed.
 */
void Column_shift_down(Column* col, Reltime* pos, Reltime* len);


/**
 * Removes all Events from the Column.
 *
 * \param col   The Column -- must not be \c NULL.
 */
void Column_clear(Column* col);


/**
 * Sets the Column length. Events located after the new end position
 * will not be removed.
 *
 * \param col   The Column -- must not be \c NULL.
 * \param len   The new Column length -- must not be \c NULL.
 */
void Column_set_length(Column* col, Reltime* len);


/**
 * Gets the Column length.
 *
 * \param col   The Column -- must not be \c NULL.
 *
 * \return   The current Column length.
 */
Reltime* Column_length(Column* col);


/**
 * Destroys an existing Column.
 *
 * \param col   The Column -- must not be \c NULL.
 */
void del_Column(Column* col);


#endif // K_COLUMN_H


