

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/common.h>

#include <kunquat/Handle.h>


kqt_Handle* handle = NULL;


void handle_setup(void)
{
    assert(handle == NULL);
    handle = kqt_new_Handle_m();
    if (handle == NULL)
    {
        fprintf(stderr, "%s\n", kqt_Handle_get_error(NULL));
        abort();
    }
}


void handle_teardown(void)
{
    assert(handle != NULL);
    kqt_del_Handle(handle);
    handle = NULL;
    return;
}


START_TEST(Nothing)
{
}
END_TEST


#define KT_VALUES(fmt, expected, actual) \
    "\n    Expected: " fmt \
    "\n    Actual:   " fmt , expected, actual


START_TEST(Initial_error_message_is_empty_string)
{
    assert(handle != NULL);
    char* error_string = kqt_Handle_get_error(handle);
    fail_unless(
            strcmp(error_string, "") == 0,
            KT_VALUES("%s", "", error_string));
}
END_TEST


Suite* Handle_suite(void)
{
    Suite* s = suite_create("Handle");

    int timeout = 4;

    TCase* tc_load = tcase_create("load");
    suite_add_tcase(s, tc_load);
    tcase_add_checked_fixture(tc_load, handle_setup, handle_teardown);
    tcase_set_timeout(tc_load, timeout);

    tcase_add_test(tc_load, Nothing);
    tcase_add_test(tc_load, Initial_error_message_is_empty_string);

    return s;
}


int main(void)
{
    Suite* suite = Handle_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


