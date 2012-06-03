

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


#include <assert.h>
#include <stdlib.h>

#include <check.h>


#ifdef K_MEM_DEBUG
#define NDEBUG
#endif


/**
 * Runs a given test suite and reports the number of failed tests.
 *
 * \param suite   The test suite -- must not be \c NULL.
 *
 * \return   The number of failed tests.
 */
int test_common_run(Suite* suite)
{
    assert(suite != NULL);
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    return fail_count;
}


