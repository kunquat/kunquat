

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


START_TEST(Handle_creation_prefers_unused_ids)
{
    kqt_Handle handles[KQT_HANDLES_MAX] = { 0 };

    kqt_Handle stored_handle = 0;

    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        kqt_Handle cur_handle = kqt_new_Handle();
        check_unexpected_error();

        handles[i] = cur_handle;

        if (i == 5)
            stored_handle = cur_handle;
        else
            kqt_del_Handle(cur_handle);

        check_unexpected_error();

        for (int k = 0; k < i; ++k)
        {
            fail_if(handles[k] == cur_handle,
                    "libkunquat reused handle %d",
                    cur_handle);
        }
    }

    assert(stored_handle != 0);
    kqt_del_Handle(stored_handle);

    check_unexpected_error();

    kqt_Handle reuse_handle = kqt_new_Handle();
    check_unexpected_error();
    fail_if(reuse_handle <= 0 || reuse_handle > KQT_HANDLES_MAX,
            "Unexpected handle ID %d after using all possible IDs",
            (int)reuse_handle);
    kqt_del_Handle(reuse_handle);
}
END_TEST


START_TEST(Do_nothing)
{
}
END_TEST


START_TEST(Initial_error_message_is_empty_string)
{
    assert(handle != 0);
    check_unexpected_error();
}
END_TEST


START_TEST(Empty_composition_has_zero_duration)
{
    assert(handle != 0);
    long long dur = kqt_Handle_get_duration(handle, songs[_i]);
    check_unexpected_error();
    fail_unless(
            dur == 0,
            "Wrong duration"
            KT_VALUES("%lld", 0, dur));
}
END_TEST


START_TEST(Default_audio_rate_is_correct)
{
    assert(handle != 0);
    long rate = kqt_Handle_get_audio_rate(handle);
    check_unexpected_error();
    fail_unless(
            rate == mixing_rates[MIXING_RATE_DEFAULT],
            "Wrong mixing rate"
            KT_VALUES("%ld", mixing_rates[MIXING_RATE_DEFAULT], rate));
}
END_TEST


START_TEST(Set_audio_rate)
{
    set_audio_rate(mixing_rates[_i]);
}
END_TEST


static Suite* Handle_suite(void)
{
    Suite* s = suite_create("Handle");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_handles = tcase_create("handles");
    suite_add_tcase(s, tc_handles);
    tcase_set_timeout(tc_handles, timeout);

    tcase_add_test(tc_handles, Handle_creation_prefers_unused_ids);

    TCase* tc_empty = tcase_create("empty");
    suite_add_tcase(s, tc_empty);
    tcase_set_timeout(tc_empty, timeout);
    tcase_add_checked_fixture(tc_empty, setup_empty, handle_teardown);

    tcase_add_test(tc_empty, Do_nothing);
    tcase_add_test(tc_empty, Initial_error_message_is_empty_string);
    tcase_add_loop_test(
            tc_empty, Empty_composition_has_zero_duration,
            0, SONG_SELECTION_COUNT);
    tcase_add_test(tc_empty, Default_audio_rate_is_correct);
    tcase_add_loop_test(
            tc_empty, Set_audio_rate,
            0, MIXING_RATE_COUNT);

    TCase* tc_render = tcase_create("render");
    suite_add_tcase(s, tc_render);
    tcase_set_timeout(tc_render, timeout);
    tcase_add_checked_fixture(tc_render, setup_empty, handle_teardown);
    tcase_add_checked_fixture(tc_render, setup_debug_instrument, NULL);

    tcase_add_test(tc_render, Do_nothing);
    tcase_add_loop_test(
            tc_render, Set_audio_rate,
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


