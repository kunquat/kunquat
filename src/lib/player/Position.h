

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_POSITION_H
#define KQT_POSITION_H


#include <mathnum/Tstamp.h>
#include <Pat_inst_ref.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Playback position.
 */
typedef struct Position
{
    int16_t      track;
    int16_t      system;
    Tstamp       pat_pos;
    Pat_inst_ref piref;
} Position;


/**
 * Initialise the Position.
 *
 * \param pos   The Position -- must not be \c NULL.
 */
void Position_init(Position* pos);


/**
 * Check the validity of the Position.
 *
 * \param pos   The Position, or \c NULL.
 *
 * \return   \c true if \a pos is valid, otherwise \c false.
 */
bool Position_is_valid(const Position* pos);


/**
 * Check the validity of the pattern position inside the Position.
 *
 * \param pos   The Position, or \c NULL.
 *
 * \return   \c true if \a pos contains a valid Pattern instance reference and
 *           row position, otherwise \c false.
 */
bool Position_has_valid_pattern_pos(const Position* pos);


#endif // KQT_POSITION_H


