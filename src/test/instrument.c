

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
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


void setup_single_pulse_without_instrument_manifest(void)
{
    assert(handle != 0);

    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"ins_00/out_00\", \"out_00\"] ]");

    set_data("ins_00/out_00/p_manifest.json", "{}");
    set_data("ins_00/p_connections.json",
            "[ [\"prc_00/C/out_00\", \"out_00\"] ]");

    set_data("p_control_map.json", "[ [0, 0] ]");
    set_data("control_00/p_manifest.json", "{}");

    set_data("ins_00/prc_00/p_manifest.json", "{}");
    set_data("ins_00/prc_00/out_00/p_manifest.json", "{}");
    set_data("ins_00/prc_00/p_prc_type.json", "\"debug\"");
    set_data("ins_00/prc_00/c/p_b_single_pulse.json", "true");

    check_unexpected_error();

    return;
}


#if 0
START_TEST(Instrument_without_manifest_is_silent)
{
    set_mix_volume(0);
    pause();

    setup_single_pulse_without_instrument_manifest();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST
#endif


START_TEST(Adding_manifest_enables_instrument)
{
    set_mix_volume(0);
    pause();

    setup_single_pulse_without_instrument_manifest();
    set_data("ins_00/p_manifest.json", "{}");
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


#if 0
START_TEST(Removing_manifest_disables_instrument)
{
    set_mix_volume(0);
    pause();

    setup_debug_instrument();
    setup_debug_single_pulse();
    set_data("ins_00/p_manifest.json", "");
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
#endif


START_TEST(Input_map_maintains_indices)
{
    set_audio_rate(220);
    set_mix_volume(0);
    pause();

    // Set up two debug instruments
    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"ins_00/out_00\", \"out_00\"],"
            "  [\"ins_01/out_00\", \"out_00\"] ]");

    set_data("p_control_map.json", "[[0, 0], [1, 1]]");
    set_data("control_00/p_manifest.json", "{}");
    set_data("control_01/p_manifest.json", "{}");

    set_data("ins_00/p_manifest.json", "{}");
    set_data("ins_00/out_00/p_manifest.json", "{}");
    set_data("ins_00/p_connections.json",
            "[ [\"prc_00/C/out_00\", \"out_00\"] ]");

    set_data("ins_00/prc_00/p_manifest.json", "{}");
    set_data("ins_00/prc_00/p_prc_type.json", "\"debug\"");
    set_data("ins_00/prc_00/out_00/p_manifest.json", "{}");

    set_data("ins_00/prc_00/c/p_b_single_pulse.json", "true");

    set_data("ins_01/p_manifest.json", "{}");
    set_data("ins_01/out_00/p_manifest.json", "{}");
    set_data("ins_01/p_connections.json",
            "[ [\"prc_00/C/out_00\", \"out_00\"] ]");

    set_data("ins_01/prc_00/p_manifest.json", "{}");
    set_data("ins_01/prc_00/p_prc_type.json", "\"debug\"");
    set_data("ins_01/prc_00/out_00/p_manifest.json", "{}");

    validate();
    check_unexpected_error();

    // Test rendering
    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\".i\", 0]");
    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();

    const int note_offset = 10;

    mix_and_fill(actual_buf, note_offset);

    kqt_Handle_fire_event(handle, 1, "[\".i\", 1]");
    kqt_Handle_fire_event(handle, 1, Note_On_55_Hz);
    check_unexpected_error();

    mix_and_fill(actual_buf + note_offset, buf_len - note_offset);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    const float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf + note_offset, 10, seq);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


Suite* Instrument_suite(void)
{
    Suite* s = suite_create("Instrument");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_general = tcase_create("general");
    suite_add_tcase(s, tc_general);
    tcase_set_timeout(tc_general, timeout);
    tcase_add_checked_fixture(tc_general, setup_empty, handle_teardown);

    //tcase_add_test(tc_general, Instrument_without_manifest_is_silent);
    tcase_add_test(tc_general, Adding_manifest_enables_instrument);
    //tcase_add_test(tc_general, Removing_manifest_disables_instrument);
    tcase_add_test(tc_general, Input_map_maintains_indices);

    return s;
}


int main(void)
{
    Suite* suite = Instrument_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


