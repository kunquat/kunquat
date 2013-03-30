

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/testing.h>


START_TEST(Out_of_mem_at_handle_creation_fails_cleanly)
{
    assert(handle == NULL);

    kqt_fake_out_of_memory(0);

    handle = kqt_new_Handle();
    fail_if(handle != NULL,
            "kqt_new_Handle returned a handle with fake out of memory");

    const char* error_msg = kqt_Handle_get_error(NULL);
    fail_if(error_msg == NULL,
            "Memory allocation failure did not give an error message");
    fail_if(strstr(error_msg, "\"MemoryError\"") == NULL,
            "Error message at memory allocation failure was not a MemoryError");
}
END_TEST


Suite* Memory_suite(void)
{
    Suite* s = suite_create("Connections");

    const int timeout = 4;

    TCase* tc_create = tcase_create("create");
    suite_add_tcase(s, tc_create);
    tcase_set_timeout(tc_create, timeout);
    //tcase_add_checked_fixture(tc_create, setup_empty, handle_teardown);

    tcase_add_test(tc_create, Out_of_mem_at_handle_creation_fails_cleanly);

    return s;
}


int main(void)
{
    Suite* suite = Memory_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}



