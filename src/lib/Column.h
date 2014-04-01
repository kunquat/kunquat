

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


#ifndef K_COLUMN_H
#define K_COLUMN_H


#include <stdbool.h>
#include <stdint.h>

#include <containers/AAtree.h>
#include <Event.h>
#include <player/Event_names.h>
#include <Streader.h>
#include <Tstamp.h>


typedef struct Event_list
{
    Event* event;
    struct Event_list* prev;
    struct Event_list* next;
} Event_list;


/**
 * Column is a container for Events in a Pattern. It contains a
 * "monophonic" section of music.
 */
typedef struct Column Column;


/**
 * Column_iter is used for retrieving Events from a Column.
 */
typedef struct Column_iter
{
    uint32_t version;
    Column* col;
    AAiter tree_iter;
    Event_list* elist;
} Column_iter;


/**
 * Creates a new Column iterator.
 *
 * \param col   The Column associated with the iterator.
 */
Column_iter* new_Column_iter(Column* col);


/**
 * Initialises a Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 */
void Column_iter_init(Column_iter* iter);


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
Event* Column_iter_get(Column_iter* iter, const Tstamp* pos);


Event_list* Column_iter_get_row(Column_iter* iter, const Tstamp* pos);


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


Event_list* Column_iter_get_next_row(Column_iter* iter);


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
Column* new_Column(const Tstamp* len);


/**
 * Creates a new Column from a textual description.
 *
 * \param sr            The Streader of the JSON input -- must not be \c NULL.
 * \param len           The length of the column. If this is \c NULL, the
 *                      length is set to INT64_MAX beats.
 * \param event_names   The Event names -- must not be \c NULL.
 *
 * \return   The new Column if successful, otherwise \c NULL.
 */
Column* new_Column_from_string(
        Streader* sr,
        const Tstamp* len,
        const Event_names* event_names);


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
 * Destroys an existing Column.
 *
 * \param col   The Column, or \c NULL.
 */
void del_Column(Column* col);


#endif // K_COLUMN_H


