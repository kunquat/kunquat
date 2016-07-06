

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


#include <init/manifest.h>

#include <debug/assert.h>
#include <string/Streader.h>

#include <stdlib.h>


static bool read_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    ignore(key);
    ignore(userdata);

    return true;
}

bool read_default_manifest(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Streader_has_data(sr))
        return false;

    return Streader_read_dict(sr, read_manifest_entry, NULL);
}


