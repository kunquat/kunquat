

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


#define buf_len 128


static void make_debug_instrument(void)
{
    set_data("p_dc_blocker_enabled.json", "false");

    set_data("au_00/p_manifest.json", "{ \"type\": \"instrument\" }");
    set_data("au_00/out_00/p_manifest.json", "{}");
    set_data("au_00/p_connections.json",
            "[ [\"proc_00/C/out_00\", \"out_00\"]"
            ", [\"proc_01/C/out_00\", \"proc_00/C/in_00\"]"
            "]");

    set_data("au_00/proc_00/p_manifest.json", "{ \"type\": \"debug\" }");
    set_data("au_00/proc_00/p_signal_type.json", "\"voice\"");
    set_data("au_00/proc_00/in_00/p_manifest.json", "{}");
    set_data("au_00/proc_00/out_00/p_manifest.json", "{}");

    set_data("au_00/proc_01/p_manifest.json", "{ \"type\": \"pitch\" }");
    set_data("au_00/proc_01/p_signal_type.json", "\"voice\"");
    set_data("au_00/proc_01/out_00/p_manifest.json", "{}");

    return;
}


START_TEST(Trivial_effect_is_identity)
{
    set_audio_rate(220);
    set_mix_volume(0);
    pause();

    set_data("p_control_map.json", "[ [0, 0] ]");
    set_data("control_00/p_manifest.json", "{}");

    make_debug_instrument();

    set_data("au_01/p_manifest.json", "{ \"type\": \"effect\" }");
    set_data("au_01/in_00/p_manifest.json", "{}");
    set_data("au_01/out_00/p_manifest.json", "{}");
    set_data("au_01/p_connections.json", "[ [\"in_00\", \"out_00\"] ]");

    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"au_00/out_00\", \"au_01/in_00\"],"
            "  [\"au_01/out_00\", \"out_00\"] ]");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, seq);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Effect_with_default_volume_dsp_is_identity)
{
    set_audio_rate(220);
    set_mix_volume(0);
    pause();

    set_data("p_control_map.json", "[ [0, 0] ]");
    set_data("control_00/p_manifest.json", "{}");

    make_debug_instrument();

    set_data("au_01/p_manifest.json", "{ \"type\": \"effect\" }");
    set_data("au_01/in_00/p_manifest.json", "{}");
    set_data("au_01/out_00/p_manifest.json", "{}");
    set_data("au_01/proc_00/p_manifest.json", "{ \"type\": \"volume\" }");
    set_data("au_01/proc_00/p_signal_type.json", "\"mixed\"");
    set_data("au_01/proc_00/in_00/p_manifest.json", "{}");
    set_data("au_01/proc_00/out_00/p_manifest.json", "{}");
    set_data("au_01/p_connections.json",
            "[ [\"in_00\", \"proc_00/C/in_00\"],"
            "  [\"proc_00/C/out_00\", \"out_00\"] ]");

    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"au_00/out_00\", \"au_01/in_00\"],"
            "  [\"au_01/out_00\", \"out_00\"] ]");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, seq);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Effect_with_double_volume_dsp_and_bypass_triples_volume)
{
    set_audio_rate(220);
    set_mix_volume(0);
    pause();

    set_data("p_control_map.json", "[ [0, 0] ]");
    set_data("control_00/p_manifest.json", "{}");

    make_debug_instrument();

    set_data("au_01/p_manifest.json", "{ \"type\": \"effect\" }");
    set_data("au_01/in_00/p_manifest.json", "{}");
    set_data("au_01/out_00/p_manifest.json", "{}");
    set_data("au_01/proc_00/p_manifest.json", "{ \"type\": \"volume\" }");
    set_data("au_01/proc_00/p_signal_type.json", "\"mixed\"");
    set_data("au_01/proc_00/in_00/p_manifest.json", "{}");
    set_data("au_01/proc_00/out_00/p_manifest.json", "{}");
    set_data("au_01/proc_00/c/p_f_volume.json", "6");
    set_data("au_01/p_connections.json",
            "[ [\"in_00\", \"out_00\"],"
            "  [\"in_00\", \"proc_00/C/in_00\"],"
            "  [\"proc_00/C/out_00\", \"out_00\"] ]");

    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"au_00/out_00\", \"au_01/in_00\"],"
            "  [\"au_01/out_00\", \"out_00\"] ]");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    float seq[] = { 3.0f, 1.5f, 1.5f, 1.5f };
    repeat_seq_local(expected_buf, 10, seq);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Connect_instrument_effect_with_unconnected_dsp_and_mix)
{
    assert(handle != 0);

    pause();

    set_data("p_control_map.json", "[ [0, 0] ]");
    set_data("control_00/p_manifest.json", "{}");

    set_data("au_00/p_manifest.json", "{ \"type\": \"instrument\" }");
    set_data("au_00/out_00/p_manifest.json", "{}");
    set_data("au_00/au_01/p_manifest.json", "{ \"type\": \"effect\" }");
    set_data("au_00/au_01/in_00/p_manifest.json", "{}");
    set_data("au_00/au_01/out_00/p_manifest.json", "{}");
    set_data("au_00/au_01/proc_00/p_manifest.json", "{ \"type\": \"volume\" }");
    set_data("au_00/au_01/proc_00/in_00/p_manifest.json", "{}");
    set_data("au_00/au_01/proc_00/out_00/p_manifest.json", "{}");
    set_data("au_00/p_connections.json",
            "[ [\"au_01/out_00\", \"out_00\"] ]");

    set_data("out_00/p_manifest.json", "{}");
    set_data("p_connections.json",
            "[ [\"au_00/out_00\", \"out_00\"] ]");

    validate();

    kqt_Handle_play(handle, 128);

    return;
}
END_TEST


static Suite* Connections_suite(void)
{
    Suite* s = suite_create("Connections");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_effects = tcase_create("effects");
    suite_add_tcase(s, tc_effects);
    tcase_set_timeout(tc_effects, timeout);
    tcase_add_checked_fixture(tc_effects, setup_empty, handle_teardown);

    tcase_add_test(tc_effects, Trivial_effect_is_identity);
    tcase_add_test(tc_effects, Effect_with_default_volume_dsp_is_identity);
    tcase_add_test(
            tc_effects,
            Effect_with_double_volume_dsp_and_bypass_triples_volume);
    tcase_add_test(
            tc_effects,
            Connect_instrument_effect_with_unconnected_dsp_and_mix);

    return s;
}


int main(void)
{
    Suite* suite = Connections_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


