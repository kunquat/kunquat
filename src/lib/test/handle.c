

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
#include <kunquat/Player.h>


kqt_Handle* handle = NULL;


#define KT_VALUES(fmt, expected, actual) \
    "\n    Expected: " fmt \
    "\n    Actual:   " fmt , expected, actual


void check_unexpected_error()
{
    char* error_string = kqt_Handle_get_error(handle);
    fail_unless(
            strcmp(error_string, "") == 0,
            "Unexpected error"
            KT_VALUES("%s", "", error_string));
    return;
}


typedef enum
{
    SUBSONG_SELECTION_FIRST,
    SUBSONG_SELECTION_LAST,
    SUBSONG_SELECTION_ALL,
    SUBSONG_SELECTION_COUNT
} Subsong_selection;


static Subsong_selection subsongs[] =
{
    [SUBSONG_SELECTION_FIRST] = 0,
    [SUBSONG_SELECTION_LAST] = KQT_SUBSONGS_MAX - 1,
    [SUBSONG_SELECTION_ALL] = -1,
};


void empty_setup(void)
{
    assert(handle == NULL);
    handle = kqt_new_Handle_m();
    if (handle == NULL)
    {
        fprintf(stderr, "Couldn't create handle:\n"
                "%s\n", kqt_Handle_get_error(NULL));
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


START_TEST(Do_nothing)
{
}
END_TEST


START_TEST(Initial_error_message_is_empty_string)
{
    assert(handle != NULL);
    char* error_string = kqt_Handle_get_error(handle);
    fail_unless(
            strcmp(error_string, "") == 0,
            KT_VALUES("%s", "", error_string));
}
END_TEST


START_TEST(Empty_composition_has_zero_duration)
{
    assert(handle != NULL);
    long long dur = kqt_Handle_get_duration(handle, subsongs[_i]);
    check_unexpected_error();
    fail_unless(
            dur == 0,
            "Wrong duration"
            KT_VALUES("%lld", 0, dur));
}
END_TEST


START_TEST(Empty_composition_renders_zero_frames)
{
    assert(handle != NULL);
    long nframes = kqt_Handle_mix(handle, 256);
    check_unexpected_error();
    fail_unless(
            nframes == 0,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", 0, nframes));
}
END_TEST


Suite* Handle_suite(void)
{
    Suite* s = suite_create("Handle");

    int timeout = 4;

    TCase* tc_empty = tcase_create("empty");
    suite_add_tcase(s, tc_empty);
    tcase_add_checked_fixture(tc_empty, empty_setup, handle_teardown);
    tcase_set_timeout(tc_empty, timeout);

    tcase_add_test(tc_empty, Do_nothing);
    tcase_add_test(tc_empty, Initial_error_message_is_empty_string);
    tcase_add_loop_test(
            tc_empty, Empty_composition_has_zero_duration,
            0, SUBSONG_SELECTION_COUNT);
    tcase_add_test(tc_empty, Empty_composition_renders_zero_frames);

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


