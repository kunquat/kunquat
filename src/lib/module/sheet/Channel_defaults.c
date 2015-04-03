

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <module/sheet/Channel_defaults.h>
#include <string/common.h>
#include <string/Streader.h>


Channel_defaults* Channel_defaults_init(Channel_defaults* chd)
{
    assert(chd != NULL);

    chd->control_id = 0;

    return chd;
}


static bool read_ch_defaults_item(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    Channel_defaults* chd = userdata;

    if (string_eq(key, "control_id"))
    {
        int64_t cid = -1;
        if (!Streader_read_int(sr, &cid))
            return false;

        if ((cid < 0) || (cid >= KQT_CONTROLS_MAX))
        {
            Streader_set_error(sr, "Invalid control ID: %" PRId64, cid);
            return false;
        }

        chd->control_id = cid;
    }
    else
    {
        Streader_set_error(sr, "Unrecognised key in channel defaults: %s", key);
        return false;
    }

    return true;
}


bool Channel_defaults_read(Channel_defaults* chd, Streader* sr)
{
    assert(chd != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Channel_defaults_init(chd);

    if (!Streader_read_dict(sr, read_ch_defaults_item, chd))
        return false;

    return true;
}


