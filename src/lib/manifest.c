

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdio.h>

#include <debug/assert.h>
#include <manifest.h>


static bool read_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    (void)sr;
    (void)key;
    (void)userdata;

    return true;
}

bool read_default_manifest(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Streader_has_data(sr))
        return false;

    return Streader_read_dict(sr, read_manifest_entry, NULL);
}


