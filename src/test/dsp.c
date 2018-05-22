

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


static void make_debug_instrument(void)
{
    set_data("p_dc_blocker_enabled.json", "[0, false]");

    set_data("au_02/proc_00/in_00/p_manifest.json", "[0, {}]");
    set_data("au_02/proc_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_02/proc_00/p_manifest.json", "[0, { \"type\": \"debug\" }]");
    set_data("au_02/proc_00/p_signal_type.json", "[0, \"voice\"]");

    set_data("au_02/proc_01/out_00/p_manifest.json", "[0, {}]");
    set_data("au_02/proc_01/p_manifest.json", "[0, { \"type\": \"pitch\" }]");
    set_data("au_02/proc_01/p_signal_type.json", "[0, \"voice\"]");

    set_data("au_02/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
    set_data("au_02/out_00/p_manifest.json", "[0, {}]");
    set_data("au_02/p_connections.json",
            "[0,"
            "[ [\"proc_00/C/out_00\", \"out_00\"]"
            ", [\"proc_01/C/out_00\", \"proc_00/C/in_00\"]"
            "]"
            "]");

    return;
}


START_TEST(Trivial_delay_is_identity)
{
    set_audio_rate(220);
    set_mix_volume(0);
    pause();

    set_data("au_03/proc_01/in_00/p_manifest.json", "[0, {}]");
    set_data("au_03/proc_01/out_00/p_manifest.json", "[0, {}]");
    set_data("au_03/proc_01/p_manifest.json", "[0, { \"type\": \"delay\" }]");
    set_data("au_03/proc_01/p_signal_type.json", "[0, \"mixed\"]");

    set_data("au_03/p_connections.json",
            "[0,"
            "[ [\"in_00\", \"proc_01/C/in_00\"], "
            "  [\"proc_01/C/out_00\", \"out_00\"] ]"
            "]");
    set_data("au_03/in_00/p_manifest.json", "[0, {}]");
    set_data("au_03/out_00/p_manifest.json", "[0, {}]");
    set_data("au_03/p_manifest.json", "[0, { \"type\": \"effect\" }]");

    make_debug_instrument();

    set_data("out_00/p_manifest.json", "[0, {}]");
    set_data("p_connections.json",
            "[0,"
            "[ [\"au_02/out_00\", \"au_03/in_00\"], "
            "  [\"au_03/out_00\", \"out_00\"] ]"
            "]");
    set_data("p_control_map.json", "[0, [ [0, 2] ]]");
    set_data("control_00/p_manifest.json", "[0, {}]");

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


static Suite* DSP_suite(void)
{
    Suite* s = suite_create("DSP");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_chorus = tcase_create("chorus");
    suite_add_tcase(s, tc_chorus);
    tcase_set_timeout(tc_chorus, timeout);
    tcase_add_checked_fixture(tc_chorus, setup_empty, handle_teardown);

    tcase_add_test(tc_chorus, Trivial_delay_is_identity);

    return s;
}


int main(void)
{
    Suite* suite = DSP_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}



