

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


#ifndef KQT_VOICE_GROUP_RESERVATIONS_H
#define KQT_VOICE_GROUP_RESERVATIONS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <kunquat/limits.h>


typedef struct Voice_group_res_entry
{
    int channel;
    uint64_t group_id;
} Voice_group_res_entry;


typedef struct Voice_group_reservations
{
    int add_pos;
    int res_count;
    Voice_group_res_entry reservations[KQT_VOICES_MAX];
} Voice_group_reservations;


void Voice_group_reservations_init(Voice_group_reservations* res);


void Voice_group_reservations_add_entry(
        Voice_group_reservations* res, int channel, uint64_t group_id);


bool Voice_group_reservations_get_clear_entry(
        Voice_group_reservations* res, int channel, uint64_t* out_group_id);


#endif // KQT_VOICE_GROUP_RESERVATIONS_H


