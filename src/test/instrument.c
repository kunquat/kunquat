

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
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


static void setup_single_pulse_without_instrument_manifest(void)
{
    assert(handle != 0);

    set_data("p_dc_blocker_enabled.json", "[0, false]");

    set_data("out_00/p_manifest.json", "[0, {}]");
    set_data("p_connections.json",
            "[0, [ [\"au_00/out_00\", \"out_00\"] ]]");

    set_data("au_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/p_connections.json",
            "[0, [ [\"proc_00/C/out_00\", \"out_00\"] ]]");

    set_data("p_control_map.json", "[0, [ [0, 0] ]]");
    set_data("control_00/p_manifest.json", "[0, {}]");

    set_data("au_00/proc_00/p_manifest.json", "[0, { \"type\": \"debug\" }]");
    set_data("au_00/proc_00/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_00/proc_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/proc_00/c/p_b_single_pulse.json", "[0, true]");

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
    set_data("au_00/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
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
    set_data("au_00/p_manifest.json", "");
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
    set_data("p_dc_blocker_enabled.json", "[0, false]");

    set_data("out_00/p_manifest.json", "[0, {}]");
    set_data("p_connections.json",
            "[0,"
            "[ [\"au_00/out_00\", \"out_00\"],"
            "  [\"au_01/out_00\", \"out_00\"] ]"
            "]");

    set_data("p_control_map.json", "[0, [[0, 0], [1, 1]]]");
    set_data("control_00/p_manifest.json", "[0, {}]");
    set_data("control_01/p_manifest.json", "[0, {}]");

    set_data("au_00/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
    set_data("au_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/p_connections.json",
            "[0, [ [\"proc_00/C/out_00\", \"out_00\"] ]]");

    set_data("au_00/proc_00/p_manifest.json", "[0, { \"type\": \"debug\" }]");
    set_data("au_00/proc_00/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_00/proc_00/out_00/p_manifest.json", "[0, {}]");

    set_data("au_00/proc_00/c/p_b_single_pulse.json", "[0, true]");

    set_data("au_01/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
    set_data("au_01/out_00/p_manifest.json", "[0, {}]");
    set_data("au_01/p_connections.json",
            "[0,"
            "[ [\"proc_00/C/out_00\", \"out_00\"]"
            ", [\"proc_01/C/out_00\", \"proc_00/C/in_00\"] ]"
            "]");

    set_data("au_01/proc_00/p_manifest.json", "[0, { \"type\": \"debug\" }]");
    set_data("au_01/proc_00/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_01/proc_00/in_00/p_manifest.json", "[0, {}]");
    set_data("au_01/proc_00/out_00/p_manifest.json", "[0, {}]");

    set_data("au_01/proc_01/p_manifest.json", "[0, { \"type\": \"pitch\" }]");
    set_data("au_01/proc_01/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_01/proc_01/out_00/p_manifest.json", "[0, {}]");

    validate();
    check_unexpected_error();

    // Test rendering
    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\".a\", 0]");
    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();

    const int note_offset = 10;

    mix_and_fill(actual_buf, note_offset);

    kqt_Handle_fire_event(handle, 1, "[\".a\", 1]");
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


START_TEST(Add_and_remove_internal_effect_and_render)
{
    set_audio_rate(220);
    set_mix_volume(0);

    setup_debug_instrument();
    setup_debug_single_pulse();

    float actual_buf[buf_len] = { 0.0f };

    // Add silence
    set_data("album/p_manifest.json", "[0, {}]");
    set_data("album/p_tracks.json", "[0, [0]]");
    set_data("song_00/p_manifest.json", "[0, {}]");
    set_data("song_00/p_order_list.json", "[0, [ [0, 0] ]]");
    set_data("pat_000/p_manifest.json", "[0, {}]");
    set_data("pat_000/p_length.json", "[0, [16, 0]]");
    set_data("pat_000/instance_000/p_manifest.json", "[0, {}]");

    // Add internal audio unit
    set_data("au_00/au_00/p_manifest.json", "[0, { \"type\": \"effect\" }]");
    set_data("au_00/au_00/in_00/p_manifest.json", "[0, {}]");
    set_data("au_00/au_00/out_00/p_manifest.json", "[0, {}]");
    set_data(
            "au_00/p_connections.json",
            "[0,"
            "[ [\"proc_00/C/out_00\", \"out_00\"]"
            ", [\"proc_00/C/out_00\", \"au_00/in_00\"]"
            ", [\"au_00/out_00\", \"out_00\"] ]"
            "]");

    // Add processor inside the internal audio unit
    set_data("au_00/au_00/proc_00/in_00/p_manifest.json", "[0, {}]");
    set_data("au_00/au_00/proc_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/au_00/proc_00/p_manifest.json", "[0, { \"type\": \"volume\" }]");
    set_data("au_00/au_00/proc_00/p_signal_type.json", "[0, \"mixed\"]");
    set_data(
            "au_00/au_00/p_connections.json",
            "[0,"
            "[ [\"in_00\", \"proc_00/C/in_00\"]"
            ", [\"proc_00/C/out_00\", \"out_00\"] ]"
            "]");

    validate();
    check_unexpected_error();

    // Test rendering
    mix_and_fill(actual_buf, buf_len);

    // Remove the internal audio unit
    set_data("au_00/p_connections.json", "[0, [ [\"proc_00/C/out_00\", \"out_00\"] ]]");
    set_data("au_00/au_00/in_00/p_manifest.json", "");
    set_data("au_00/au_00/out_00/p_manifest.json", "");
    set_data("au_00/au_00/p_connections.json", "");
    set_data("au_00/au_00/p_manifest.json", "");
    set_data("au_00/au_00/proc_00/in_00/p_manifest.json", "");
    set_data("au_00/au_00/proc_00/out_00/p_manifest.json", "");
    set_data("au_00/au_00/proc_00/p_manifest.json", "");
    set_data("au_00/au_00/proc_00/p_signal_type.json", "");

    validate();
    check_unexpected_error();

    // Test rendering again
    kqt_Handle_set_position(handle, 0, 0);
    mix_and_fill(actual_buf, buf_len);
}
END_TEST


START_TEST(Read_audio_unit_control_vars)
{
    setup_single_pulse_without_instrument_manifest();
    set_data("au_00/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
    validate();
    check_unexpected_error();

    set_data(
            "au_00/p_control_vars.json",
            "[0,"
            "[[\"float\", \"test\", 0.0, [0.0, 1.0],"
                " [[\"proc_00\", \"target\", \"float\", 0.0, 1.0]]]]"
            "]");
}
END_TEST


static Suite* Instrument_suite(void)
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
    tcase_add_test(tc_general, Add_and_remove_internal_effect_and_render);
    tcase_add_test(tc_general, Read_audio_unit_control_vars);

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


