

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


#include <kunquat/testing.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdint.h>


void kqt_fake_out_of_memory(long steps)
{
    memory_fake_out_of_memory((int32_t)clamp(steps, INT32_MIN, INT32_MAX));
    return;
}


long kqt_get_memory_alloc_count(void)
{
    return memory_get_alloc_count();
}


void kqt_suppress_assert_messages(void)
{
    assert_suppress_messages();
    return;
}


