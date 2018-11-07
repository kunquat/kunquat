

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
    int32_t size;
    int init_sub_count;
    int sub_count;
    int32_t const_start[WORK_BUFFER_SUB_COUNT_MAX];
    bool is_final[WORK_BUFFER_SUB_COUNT_MAX]; // TODO: make more compact
    void* contents;
};


#endif // KQT_WORK_BUFFER_PRIVATE_H


