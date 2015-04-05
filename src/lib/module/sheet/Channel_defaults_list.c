

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


#include <stdint.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <memory.h>
#include <module/sheet/Channel_defaults.h>
#include <module/sheet/Channel_defaults_list.h>
#include <string/Streader.h>


static bool read_ch_defaults_item(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);

    Channel_defaults_list* cdl = userdata;

    if (index >= KQT_CHANNELS_MAX)
    {
        Streader_set_error(sr, "Too many channel descriptions");
        return false;
    }

    return Channel_defaults_read(&cdl->ch_defaults[index], sr);
}


Channel_defaults_list* new_Channel_defaults_list(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Channel_defaults_list* cdl = memory_alloc_item(Channel_defaults_list);
    if (cdl == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for channel defaults");
        return NULL;
    }

    if (!Streader_has_data(sr))
    {
        for (int ch = 0; ch < KQT_CHANNELS_MAX; ++ch)
            Channel_defaults_init(&cdl->ch_defaults[ch]);

        return cdl;
    }

    if (!Streader_read_list(sr, read_ch_defaults_item, cdl))
    {
        del_Channel_defaults_list(cdl);
        return NULL;
    }

    return cdl;
}


const Channel_defaults* Channel_defaults_list_get(
        const Channel_defaults_list* cdl, int32_t ch_index)
{
    assert(cdl != NULL);
    assert(ch_index >= 0);
    assert(ch_index < KQT_CHANNELS_MAX);

    return &cdl->ch_defaults[ch_index];
}


void del_Channel_defaults_list(Channel_defaults_list* cdl)
{
    if (cdl == NULL)
        return;

    memory_free(cdl);

    return;
}


