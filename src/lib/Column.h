

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <Tstamp.h>
#include <Event.h>
#include <Event_names.h>
#include <AAtree.h>


typedef struct Event_list
{
    Event* event;
    bool copy;
    struct Event_list* prev;
    struct Event_list* next;
} Event_list;


/**
 * Column is a container for Events in a Pattern. It typically contains a
 * "monophonic" section of music, or global Events.
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
 * Creates a new auxiliary global Column.
 *
 * \param old_aux   The old auxiliary global Column, or \c NULL if not
 *                  applicable.
 * \param mod_col   The Column with the new Events -- must not be \c NULL.
 * \param index     The Column index -- must be >= \c 0 and
 *                  < \c KQT_COLUMNS_MAX.
 */
Column* new_Column_aux(Column* old_aux, Column* mod_col, int index);


/**
 * Creates a new Column from a textual description.
 *
 * \param len              The length of the column. If this is \c NULL, the
 *                         length is set to INT64_MAX beats.
 * \param str              The textual description -- must not be \c NULL.
 * \param locations        Pattern location info -- must not be \c NULL.
 * \param locations_iter   The iterator for \a locations -- must not be
 *                         \c NULL.
 * \param event_names      The Event names -- must not be \c NULL.
 * \param state            The Read state -- must not be \c NULL.
 *
 * \return   The new Column if successful, otherwise \c NULL. \a state
 *           will _not_ be updated if memory allocation failed.
 */
Column* new_Column_from_string(const Tstamp* len,
                               char* str,
//                               bool is_global,
                               AAtree* locations,
                               AAiter* locations_iter,
                               const Event_names* event_names,
                               Read_state* state);


/**
 * Updates location info in the Column.
 *
 * \param col              The Column -- must not be \c NULL.
 * \param locations        Pattern location info -- must not be \c NULL.
 * \param locations_iter   The iterator for \a locations -- must not be
 *                         \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Column_update_locations(Column* col,
                             AAtree* locations,
                             AAiter* locations_iter);


/**
 * Parses a Column.
 *
 * \param col         The Column -- must not be \c NULL.
 * \param str         The textual description -- must not be \c NULL.
 * \param is_global   \c true if and only if \a col is a global Column.
 * \param state       The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
//bool Column_parse(Column* col, char* str, bool is_global, Read_state* state);


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


