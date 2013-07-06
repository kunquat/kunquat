

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


START_TEST(Do_nothing)
{
}
END_TEST


START_TEST(Initial_error_message_is_empty_string)
{
    assert(handle != NULL);
    check_unexpected_error();
}
END_TEST


START_TEST(Empty_composition_has_zero_duration)
{
    assert(handle != NULL);
    long long dur = kqt_Handle_get_duration(handle, songs[_i]);
    check_unexpected_error();
    fail_unless(
            dur == 0,
            "Wrong duration"
            KT_VALUES("%lld", 0, dur));
}
END_TEST


START_TEST(Default_mixing_rate_is_correct)
{
    assert(handle != NULL);
    long rate = kqt_Handle_get_mixing_rate(handle);
    check_unexpected_error();
    fail_unless(
            rate == mixing_rates[MIXING_RATE_DEFAULT],
            "Wrong mixing rate"
            KT_VALUES("%ld", mixing_rates[MIXING_RATE_DEFAULT], rate));
}
END_TEST


START_TEST(Set_mixing_rate)
{
    set_mixing_rate(mixing_rates[_i]);
}
END_TEST


#define buf_len 128


Suite* Handle_suite(void)
{
    Suite* s = suite_create("Handle");

    int timeout = 4;

    TCase* tc_empty = tcase_create("empty");
    suite_add_tcase(s, tc_empty);
    tcase_set_timeout(tc_empty, timeout);
    tcase_add_checked_fixture(tc_empty, setup_empty, handle_teardown);

    tcase_add_test(tc_empty, Do_nothing);
    tcase_add_test(tc_empty, Initial_error_message_is_empty_string);
    tcase_add_loop_test(
            tc_empty, Empty_composition_has_zero_duration,
            0, SONG_SELECTION_COUNT);
    tcase_add_test(tc_empty, Default_mixing_rate_is_correct);
    tcase_add_loop_test(
            tc_empty, Set_mixing_rate,
            0, MIXING_RATE_COUNT);

    TCase* tc_render = tcase_create("render");
    suite_add_tcase(s, tc_render);
    tcase_set_timeout(tc_render, timeout);
    tcase_add_checked_fixture(tc_render, setup_empty, handle_teardown);
    tcase_add_checked_fixture(tc_render, setup_debug_instrument, NULL);

    tcase_add_test(tc_render, Do_nothing);
    tcase_add_loop_test(
            tc_render, Set_mixing_rate,
            0, MIXING_RATE_COUNT);

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


