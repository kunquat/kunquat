

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <kunquat/limits.h>
#include <Pattern_location.h>
#include <xassert.h>
#include <xmemory.h>


Pattern_location* new_Pattern_location(int song, Pat_inst_ref* piref)
{
    assert(song >= 0);
    assert(song < KQT_SONGS_MAX);
    assert(piref != NULL);
    Pattern_location* loc = xalloc(Pattern_location);
    if (loc == NULL)
    {
        return NULL;
    }
    loc->song = song;
    loc->piref = *piref;
    return loc;
}


int Pattern_location_cmp(const Pattern_location* loc1,
                         const Pattern_location* loc2)
{
    assert(loc1 != NULL);
    assert(loc2 != NULL);
    if (loc1->song < loc2->song)
    {
        return -1;
    }
    else if (loc1->song > loc2->song)
    {
        return 1;
    }
    return Pat_inst_ref_cmp(&loc1->piref, &loc2->piref);
}


