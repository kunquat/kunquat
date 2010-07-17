

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <kunquat/Info.h>

#include <Handle_private.h>
#include <Subsong.h>
#include <Subsong_table.h>
#include <Song.h>
#include <xassert.h>


int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong)
{
    check_handle(handle, -1);
    if (subsong < -1 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Invalid subsong number: %d", subsong);
        return -1;
    }
    assert(handle->song != NULL);
    if (subsong == -1)
    {
        int total = 0;
        for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
        {
            Subsong* ss = Subsong_table_get(Song_get_subsongs(handle->song), i);
            if (ss == NULL)
            {
                break;
            }
            total += Subsong_get_length(ss);
        }
        return total;
    }
    Subsong* ss = Subsong_table_get(Song_get_subsongs(handle->song), subsong);
    if (ss == NULL)
    {
        return 0;
    }
    return Subsong_get_length(ss);
}


