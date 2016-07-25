

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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


#include <stdint.h>


struct Work_buffer
{
    int32_t size;
    int32_t const_start;
    void* contents;
};


#endif // KQT_WORK_BUFFER_PRIVATE_H


