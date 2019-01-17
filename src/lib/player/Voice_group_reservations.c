

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice_group_reservations.h>

#include <debug/assert.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


void Voice_group_reservations_init(Voice_group_reservations* res)
{
    rassert(res != NULL);

    res->add_pos = 0;
    res->res_count = 0;

    return;
}


void Voice_group_reservations_add_entry(
        Voice_group_reservations* res, int channel, uint64_t group_id)
{
    rassert(res != NULL);
    rassert(channel >= 0);
    rassert(channel < KQT_CHANNELS_MAX);

    Voice_group_res_entry* entry = &res->reservations[res->add_pos];
    entry->channel = channel;
    entry->group_id = group_id;

    ++res->add_pos;
    if (res->add_pos >= KQT_VOICES_MAX)
        res->add_pos = 0;

    if (res->res_count < KQT_VOICES_MAX)
        ++res->res_count;

    return;
}


bool Voice_group_reservations_get_clear_entry(
        Voice_group_reservations* res, int channel, uint64_t* out_group_id)
{
    rassert(res != NULL);
    rassert(channel >= 0);
    rassert(channel < KQT_CHANNELS_MAX);
    rassert(out_group_id != NULL);

    // TODO: this may need additional sync

    for (int i = 0; i < res->res_count; ++i)
    {
        Voice_group_res_entry* entry = &res->reservations[i];
        if (entry->channel == channel)
        {
            entry->channel = -1;
            *out_group_id = entry->group_id;
            return true;
        }
    }

    return false;
}


