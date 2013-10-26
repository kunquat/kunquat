

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


#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


#define buf_len 128


void setup_single_pulse_without_generator_manifest(void)
{
    assert(handle != 0);

    set_data("p_connections.json",
            "[ [\"ins_00/out_00\", \"out_00\"] ]");

    set_data("ins_00/p_manifest.json", "{}");
    set_data("ins_00/p_connections.json",
            "[ [\"gen_00/C/out_00\", \"out_00\"] ]");

    set_data("ins_00/gen_00/p_gen_type.json", "\"debug\"");
    set_data("ins_00/gen_00/c/p_single_pulse.jsonb", "true");

    validate();

    check_unexpected_error();

    return;
}


START_TEST(Generator_without_manifest_is_silent)
{
    set_mix_volume(0);
    pause();

    setup_single_pulse_without_generator_manifest();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Adding_manifest_enables_generator)
{
    set_mix_volume(0);
    pause();

    setup_single_pulse_without_generator_manifest();
    set_data("ins_00/gen_00/p_manifest.json", "{}");
    validate();
    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Removing_manifest_disables_generator)
{
    set_mix_volume(0);
    pause();

    setup_debug_instrument();
    setup_debug_single_pulse();
    set_data("ins_00/gen_00/p_manifest.json", "");
    validate();
    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


Suite* Generator_suite(void)
{
    Suite* s = suite_create("Generator");

    const int timeout = 4;

    TCase* tc_general = tcase_create("general");
    suite_add_tcase(s, tc_general);
    tcase_set_timeout(tc_general, timeout);
    tcase_add_checked_fixture(tc_general, setup_empty, handle_teardown);

    tcase_add_test(tc_general, Generator_without_manifest_is_silent);
    tcase_add_test(tc_general, Adding_manifest_enables_generator);
    tcase_add_test(tc_general, Removing_manifest_disables_generator);

    return s;
}


int main(void)
{
    Suite* suite = Generator_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


