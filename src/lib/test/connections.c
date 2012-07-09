

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


#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


START_TEST(Connect_instrument_effect_with_unconnected_dsp_and_mix)
{
    assert(handle != NULL);

    pause();

    set_data("ins_00/kqtiXX/eff_00/kqteXX/dsp_00/kqtdXX/p_dsp_type.json",
            "\"volume\"");
    set_data("p_connections.json",
            "[ [\"ins_00/kqtiXX/out_00\", \"out_00\"] ]");
    set_data("ins_00/kqtiXX/p_connections.json",
            "[ [\"eff_00/kqteXX/out_00\", \"out_00\"] ]");
    kqt_Handle_mix(handle, 128);

    return;
}
END_TEST


Suite* Connections_suite(void)
{
    Suite* s = suite_create("Connections");

    int timeout = 4;

    TCase* tc_effects = tcase_create("effects");
    suite_add_tcase(s, tc_effects);
    tcase_set_timeout(tc_effects, timeout);
    tcase_add_checked_fixture(tc_effects, setup_empty, handle_teardown);

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


