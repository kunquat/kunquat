

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


#include <stdint.h>
#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <transient/Player.h>
#include <Handle_private.h>


static Player* player = NULL;


void setup_player()
{
    assert(player == NULL);
    setup_empty();
    const Module* module = Handle_get_module(handle);
    player = new_Player(module);
    fail_if(player == NULL, "Player creation failed");
    return;
}


void player_teardown()
{
    assert(player != NULL);
    del_Player(player);
    player = NULL;
    handle_teardown();
    return;
}


START_TEST(Create_player)
{
}
END_TEST


START_TEST(Empty_composition_renders_zero_frames)
{
    assert(player != NULL);
    Player_play(player, 256);
    const int32_t nframes = Player_get_frames_available(player);
    fail_unless(
            nframes == 0,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", 0, nframes));
}
END_TEST


Suite* Player_suite(void)
{
    Suite* s = suite_create("Player");

    const int timeout = 4;

    TCase* tc_render = tcase_create("render");
    suite_add_tcase(s, tc_render);
    tcase_set_timeout(tc_render, timeout);
    tcase_add_checked_fixture(tc_render, setup_player, player_teardown);

    tcase_add_test(tc_render, Create_player);
    tcase_add_test(tc_render, Empty_composition_renders_zero_frames);

    return s;
}


int main(void)
{
    Suite* suite = Player_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


