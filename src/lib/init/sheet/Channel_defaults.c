

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Channel_defaults.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <string/common.h>
#include <string/Streader.h>
#include <string/var_name.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Channel_defaults* Channel_defaults_init(Channel_defaults* chd)
{
    rassert(chd != NULL);

    chd->control_num = 0;
    memset(chd->init_expr, '\0', KQT_VAR_NAME_MAX);

    return chd;
}


static bool read_ch_defaults_item(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

    Channel_defaults* chd = userdata;

    if (string_eq(key, "control"))
    {
        int64_t cnum = -1;
        if (!Streader_read_int(sr, &cnum))
            return false;

        if ((cnum < 0) || (cnum >= KQT_CONTROLS_MAX))
        {
            Streader_set_error(sr, "Invalid control number: %" PRId64, cnum);
            return false;
        }

        chd->control_num = (int)cnum;
    }
    else if (string_eq(key, "init_expr"))
    {
        char init_expr[KQT_VAR_NAME_MAX + 1] = "";

        if (!Streader_read_string(sr, KQT_VAR_NAME_MAX + 1, init_expr))
            return false;
        if ((init_expr[0] != '\0') && !is_valid_var_name(init_expr))
        {
            Streader_set_error(sr, "Illegal initial expression %s");
            return false;
        }

        strcpy(chd->init_expr, init_expr);
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
    rassert(chd != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Channel_defaults_init(chd);

    if (!Streader_read_dict(sr, read_ch_defaults_item, chd))
        return false;

    return true;
}


