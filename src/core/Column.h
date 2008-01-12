

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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

#include <Reltime.h>
#include <Event.h>


typedef struct AAnode
{
	int level;
	Event* event;
	struct AAnode* parent;
	struct AAnode* left;
	struct AAnode* right;
} AAnode;


typedef struct AAtree
{
	AAnode* nil;
	AAnode* root;
} AAtree;


typedef struct Column
{
	Reltime len;
	AAnode* last;
	AAtree events;
} Column;


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
 * Gets an Event from the Column.
 *
 * The first Event with position greater than or equal to the given position
 * will be returned.
 *
 * \param col   The Column -- must not be \c NULL.
 * \param pos   The position of the Event -- must not be \c NULL.
 *
 * \return   The Event if one exists, otherwise \c NULL.
 */
Event* Column_get(Column* col, const Reltime* pos);


/**
 * Gets the Event next to the previous Event retrieved from the Column.
 *
 * If not preceded by a successful call to Column_get(), \c NULL will be
 * returned.
 *
 * \param col   The Column -- must not be \c NULL.
 *
 * \return   The Event if one exists, otherwise \c NULL.
 */
Event* Column_get_next(Column* col);


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


