

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


#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/testing.h>


START_TEST(Handle_refuses_to_render_unvalidated_module)
{
    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [16, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    long mixed = kqt_Handle_mix(handle, 16);

    fail_if(mixed > 0,
            "kqt_Handle_mix rendered %ld frames of unvalidated music",
            mixed);

    const char* error_msg = kqt_Handle_get_error(handle);
    fail_if(error_msg == NULL,
            "Attempting to render unvalidated music did not give an error"
            " message");
    fail_if(strstr(error_msg, "\"ArgumentError\"") == NULL,
            "Error message on missing validation was not an ArgumentError");
    fail_if(strstr(error_msg, "valid") == NULL,
            "Error message on missing validation does not contain substring"
            " \"valid\"");
}
END_TEST


Suite* Validation_suite(void)
{
    Suite* s = suite_create("Validation");

    const int timeout = 4;

    TCase* tc_refuse = tcase_create("refuse");
    suite_add_tcase(s, tc_refuse);
    tcase_set_timeout(tc_refuse, timeout);
    tcase_add_checked_fixture(tc_refuse, setup_empty, handle_teardown);

    tcase_add_test(tc_refuse, Handle_refuses_to_render_unvalidated_module);

    return s;
}


int main(void)
{
    Suite* suite = Validation_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}



