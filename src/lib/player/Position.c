

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


#include <player/Position.h>

#include <debug/assert.h>

#include <stdlib.h>


void Position_init(Position* pos)
{
    rassert(pos != NULL);

    pos->track = -1;
    pos->system = 0;
    Tstamp_set(&pos->pat_pos, 0, 0);
    pos->piref.pat = -1;
    pos->piref.inst = 0;

    return;
}


bool Position_is_valid(const Position* pos)
{
    return
        (pos != NULL) &&
        (pos->track >= 0) &&
        (pos->track < KQT_TRACKS_MAX) &&
        (pos->system >= 0) &&
        (pos->system < KQT_SYSTEMS_MAX) &&
        Position_has_valid_pattern_pos(pos);
}


bool Position_has_valid_pattern_pos(const Position* pos)
{
    return (pos != NULL) &&
        (Tstamp_cmp(&pos->pat_pos, TSTAMP_AUTO) >= 0) &&
        (pos->piref.pat >= -1) &&
        (pos->piref.pat < KQT_PATTERNS_MAX) &&
        (pos->piref.inst >= -1) &&
        (pos->piref.inst < KQT_PAT_INSTANCES_MAX);
}


