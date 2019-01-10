

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CGITER_H
#define KQT_CGITER_H


#include <decl.h>
#include <init/Module.h>
#include <init/sheet/Column.h>
#include <player/Position.h>

#include <stdbool.h>
#include <stdlib.h>


// TODO: define proper interface in Column
typedef struct Trigger_row
{
    Trigger_list* head;
} Trigger_row;


/**
 * Iterates over triggers in column groups.
 */
typedef struct Cgiter
{
    const Module* module;
    int col_index;

    Position pos;
    Trigger_row cur_tr; // TODO: remove

    bool row_returned;

    bool has_finished;
    bool is_pattern_playback_state;
} Cgiter;


/**
 * Initialise Cgiter.
 *
 * \param cgiter      The Cgiter -- must not be \c NULL.
 * \param module      The Module -- must not be \c NULL.
 * \param col_index   The column index -- must be >= \c 0 and
 *                    < \c KQT_COLUMNS_MAX.
 */
void Cgiter_init(Cgiter* cgiter, const Module* module, int col_index);


/**
 * Reset the Cgiter.
 *
 * \param cgiter      The Cgiter -- must not be \c NULL.
 * \param start_pos   The starting position -- must be valid in normal playback
 *                    mode or contain valid pattern position in pattern
 *                    playback mode.
 */
void Cgiter_reset(Cgiter* cgiter, const Position* start_pos);


/**
 * Return trigger row at the current Cgiter position.
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
 * Get distance to the next local breakpoint following the current Cgiter position.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 * \param dist     Address where the distance will be stored -- must be valid.
 *                 NOTE: The passed value is used to determine maximum
 *                 distance to be searched.
 *
 * \return   \c true if \a dist was modified, otherwise \c false.
 */
bool Cgiter_get_local_bp_dist(const Cgiter* cgiter, Tstamp* dist);


/**
 * Get distance to the next global breakpoint following the current Cgiter position.
 *
 * \param cgiter        The Cgiter -- must not be \c NULL.
 * \param bind          The Bind -- must not be \c NULL.
 * \param event_names   The Event names -- must not be \c NULL.
 * \param dist          Address where the distance will be stored -- must be valid.
 *                      NOTE: The passed value is used to determine maximum
 *                      distance to be searched.
 *
 * \return   \c true if \a dist was modified, otherwise \c false.
 */
bool Cgiter_get_global_bp_dist(
        const Cgiter* cgiter,
        const Bind* bind,
        const Event_names* event_names,
        Tstamp* dist);


/**
 * Move the iterator forwards.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 * \param dist     The distance to be moved -- must be valid and should not
 *                 point further than the next trigger row.
 */
void Cgiter_move(Cgiter* cgiter, const Tstamp* dist);


/**
 * Tell whether the Cgiter has reached the end.
 *
 * \param cgiter   The Cgiter -- must not be \c NULL.
 *
 * \return   \c true if end has been reached, otherwise \c false.
 */
bool Cgiter_has_finished(const Cgiter* cgiter);


#endif // KQT_CGITER_H


