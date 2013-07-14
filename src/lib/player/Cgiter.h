

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CGITER_H
#define K_CGITER_H


#include <stdbool.h>

#include <Column.h>
#include <Module.h>
#include <player/Position.h>


// TODO: define proper interface in Column
typedef struct Trigger_row
{
    Event_list* head;
} Trigger_row;


/**
 * Iterates over triggers in column groups.
 */
typedef struct Cgiter
{
    const Module* module;
    int col_index;

    Position pos;
    Column_iter citer;
    Trigger_row cur_tr; // TODO: remove

    bool row_returned;
} Cgiter;


/**
 * Initialises Cgiter.
 *
 * \param cgiter      The Cgiter -- must not be \c NULL.
 * \param module      The Module -- must not be \c NULL.
 * \param col_index   The column index -- must be >= \c 0 and
 *                    < \c KQT_COLUMNS_MAX.
 */
void Cgiter_init(Cgiter* cgiter, const Module* module, int col_index);


/**
 * Resets the Cgiter.
 *
 * \param cgiter      The Cgiter -- must not be \c NULL.
 * \param start_pos   The starting position -- must be valid.
 */
void Cgiter_reset(Cgiter* cgiter, const Position* start_pos);


/**
 * Returns trigger row at the current Cgiter position.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 *
 * \return   The trigger row if one exists, otherwise \c NULL.
 */
const Trigger_row* Cgiter_get_trigger_row(Cgiter* cgiter);


/**
 * Allow a previously returned trigger row to be returned again.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL and must have the
 *                 returned flag set.
 */
void Cgiter_clear_returned_status(Cgiter* cgiter);


/**
 * Gets distance to the next breakpoint following the current Cgiter position.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 * \param dist     Address where the distance will be stored -- must be valid.
 *                 NOTE: The passed value is used to determine maximum
 *                 distance to be searched.
 *
 * \return   \c true if \a dist was modified, otherwise \c false.
 */
bool Cgiter_peek(Cgiter* cgiter, Tstamp* dist);


/**
 * Moves the iterator forwards.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 * \param dist     The distance to be moved -- must be valid and should not
 *                 point further than the next trigger row.
 */
void Cgiter_move(Cgiter* cgiter, const Tstamp* dist);


/**
 * Tells whether the Cgiter has reached the end.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 *
 * \return   \c true if end has been reached, otherwise \c false.
 */
bool Cgiter_has_finished(const Cgiter* cgiter);


#endif // K_CGITER_H


