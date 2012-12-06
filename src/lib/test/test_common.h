

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KT_TEST_COMMON_H
#define KT_TEST_COMMON_H


#include <assert.h>
#include <stdlib.h>

#include <check.h>


#ifdef K_MEM_DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif


#define KT_VALUES(fmt, expected, actual) \
    "\n    Expected: " fmt \
    "\n      Actual: " fmt , expected, actual


#endif // KT_TEST_COMMON_H


