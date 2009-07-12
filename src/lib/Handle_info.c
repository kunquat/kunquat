

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>

#include <kunquat/Info.h>

#include <Handle_private.h>
#include <Subsong.h>
#include <Order.h>
#include <Song.h>


int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong)
{
    if (!handle_is_valid(handle))
    {
        kqt_Handle_set_error(NULL,
                "kqt_Handle_get_subsong_length: Invalid Kunquat Handle: %p", (void*)handle);
        return -1;
    }
    if (subsong < -1 || subsong >= KQT_SUBSONGS_MAX)
    {
        kqt_Handle_set_error(handle, "Invalid subsong number: %d", subsong);
        return -1;
    }
    assert(handle->song != NULL);
    if (subsong == -1)
    {
        int total = 0;
        for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
        {
            Subsong* ss = Order_get_subsong(Song_get_order(handle->song), i);
            if (ss == NULL)
            {
                break;
            }
            total += Subsong_get_length(ss);
        }
        return total;
    }
    Subsong* ss = Order_get_subsong(Song_get_order(handle->song), subsong);
    if (ss == NULL)
    {
        return 0;
    }
    return Subsong_get_length(ss);
}


