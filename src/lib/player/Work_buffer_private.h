

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_WORK_BUFFER_PRIVATE_H
#define KQT_WORK_BUFFER_PRIVATE_H


#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>


struct Work_buffer
{
    void* contents;
    int32_t size;
    int32_t const_start[WORK_BUFFER_SUB_COUNT_MAX];
    uint8_t init_sub_count : 4;
    uint8_t sub_count : 4;
    uint8_t is_valid : 4; // TODO: use ranges?
    uint8_t is_final : 4;
};


#endif // KQT_WORK_BUFFER_PRIVATE_H


