

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
    Player_reset(player);
    Player_play(player, 256);
    const int32_t nframes = Player_get_frames_available(player);
    fail_unless(
            nframes == 0,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", 0, nframes));
    fail_unless(
            Player_has_stopped(player),
            "Player did not reach end of composition");
}
END_TEST


START_TEST(Empty_pattern_contains_silence)
{
    set_mixing_rate(mixing_rates[_i]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[_i]),
            "Could not set player audio rate");

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [16, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    validate();

    Player_reset(player);

    const long expected_length = 8 * mixing_rates[_i];
    int32_t actual_length = 0;

    while (!Player_has_stopped(player))
    {
        Player_play(player, 4096);
        int32_t nframes = Player_get_frames_available(player);

        actual_length += nframes;

        // Don't want to spend too much time on this...
        if (_i == MIXING_RATE_LOW)
        {
            float* bufs[] =
            {
                Player_get_audio(player, 0),
                Player_get_audio(player, 1),
            };
            fail_if(bufs[0] == NULL,
                    "Player_get_audio did not return a buffer");
            fail_if(bufs[0] == NULL,
                    "Player_get_audio did not return a buffer");

            float expected_buf[128] = { 0.0f };
            assert(nframes <= 128);
            check_buffers_equal(expected_buf, bufs[0], nframes, 0.0f);
            check_buffers_equal(expected_buf, bufs[1], nframes, 0.0f);
        }
    }

    fail_unless(actual_length == expected_length,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", expected_length, (long)actual_length));
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
    tcase_add_loop_test(
            tc_render, Empty_pattern_contains_silence,
            0, MIXING_RATE_COUNT);

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


