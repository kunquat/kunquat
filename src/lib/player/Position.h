

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


#ifndef K_POSITION_H
#define K_POSITION_H


#include <stdbool.h>
#include <stdint.h>

#include <Pat_inst_ref.h>
#include <Tstamp.h>


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
 * Initialises Position.
 *
 * \param pos   The Position -- must not be \c NULL.
 */
void Position_init(Position* pos);


/**
 * Checks the validity of the Position.
 *
 * \param pos   The Position, or \c NULL.
 *
 * \return   \c true if \a pos is valid, otherwise \c false.
 */
bool Position_is_valid(const Position* pos);


#endif // K_POSITION_H


