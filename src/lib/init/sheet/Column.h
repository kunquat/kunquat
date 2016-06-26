

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


#ifndef KQT_COLUMN_H
#define KQT_COLUMN_H


#include <containers/AAtree.h>
#include <init/sheet/Trigger.h>
#include <mathnum/Tstamp.h>
#include <player/Event_names.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Trigger_list
{
    Trigger* trigger;
    struct Trigger_list* prev;
    struct Trigger_list* next;
} Trigger_list;


/**
 * Column is a container for Triggers in a Pattern. It contains a
 * "monophonic" section of music.
 */
typedef struct Column Column;


/**
 * Column_iter is used for retrieving Triggers from a Column.
 */
typedef struct Column_iter
{
    uint32_t version;
    Column* col;
    AAiter tree_iter;
    Trigger_list* trlist;
} Column_iter;


/**
 * Create a new Column iterator.
 *
 * \param col   The Column associated with the iterator, or \c NULL if memory
 *              allocation failed.
 */
Column_iter* new_Column_iter(Column* col);


/**
 * Initialise a Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 */
void Column_iter_init(Column_iter* iter);


/**
 * Change the Column associated with the Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 * \param col    The Column -- must not be \c NULL.
 */
void Column_iter_change_col(Column_iter* iter, Column* col);


/**
 * Get a Trigger from the Column.
 *
 * The first Trigger with position greater than or equal to the given position
 * will be returned.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 * \param pos    The position of the Trigger -- must not be \c NULL.
 *
 * \return   The Trigger if one exists, otherwise \c NULL.
 */
Trigger* Column_iter_get(Column_iter* iter, const Tstamp* pos);


Trigger_list* Column_iter_get_row(Column_iter* iter, const Tstamp* pos);


/**
 * Get the Trigger next to the previous Trigger retrieved from the Column.
 *
 * If not preceded by a successful call to Column_iter_get(), \c NULL will be
 * returned.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 *
 * \return   The Trigger if one exists, otherwise \c NULL.
 */
Trigger* Column_iter_get_next(Column_iter* iter);


Trigger_list* Column_iter_get_next_row(Column_iter* iter);


/**
 * Destroy an existing Column iterator.
 *
 * \param iter   The Column iterator -- must not be \c NULL.
 */
void del_Column_iter(Column_iter* iter);


/**
 * Create a new Column.
 *
 * \param length   The length of the column. If this is \c NULL, the length is
 *                 set to INT64_MAX beats.
 *
 * \return   The new Column if successful, or \c NULL if memory allocation
 *           failed.
 */
Column* new_Column(const Tstamp* len);


/**
 * Create a new Column from a textual description.
 *
 * \param sr            The Streader of the JSON input -- must not be \c NULL.
 * \param len           The length of the column. If this is \c NULL, the
 *                      length is set to INT64_MAX beats.
 * \param event_names   The Event names -- must not be \c NULL.
 *
 * \return   The new Column if successful, otherwise \c NULL.
 */
Column* new_Column_from_string(
        Streader* sr, const Tstamp* len, const Event_names* event_names);


/**
 * Insert a new Trigger into the Column.
 *
 * If other Triggers are already located at the target position, the new Trigger
 * will be placed after these Triggers.
 *
 * \param col       The Column -- must not be \c NULL.
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Column_ins(Column* col, Trigger* trigger);


/**
 * Destroy an existing Column.
 *
 * \param col   The Column, or \c NULL.
 */
void del_Column(Column* col);


#endif // KQT_COLUMN_H


