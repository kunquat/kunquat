

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


START_TEST(Do_nothing)
{
}
END_TEST


START_TEST(Initial_error_message_is_empty_string)
{
    assert(handle != NULL);
    check_unexpected_error();
}
END_TEST


START_TEST(Empty_composition_has_zero_duration)
{
    assert(handle != NULL);
    long long dur = kqt_Handle_get_duration(handle, songs[_i]);
    check_unexpected_error();
    fail_unless(
            dur == 0,
            "Wrong duration"
            KT_VALUES("%lld", 0, dur));
}
END_TEST


START_TEST(Default_mixing_rate_is_correct)
{
    assert(handle != NULL);
    long rate = kqt_Handle_get_mixing_rate(handle);
    check_unexpected_error();
    fail_unless(
            rate == mixing_rates[MIXING_RATE_DEFAULT],
            "Wrong mixing rate"
            KT_VALUES("%ld", mixing_rates[MIXING_RATE_DEFAULT], rate));
}
END_TEST


START_TEST(Empty_composition_renders_zero_frames)
{
    assert(handle != NULL);
    long nframes = kqt_Handle_mix(handle, 256);
    check_unexpected_error();
    fail_unless(
            nframes == 0,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", 0, nframes));
}
END_TEST


START_TEST(Set_mixing_rate)
{
    set_mixing_rate(mixing_rates[_i]);
}
END_TEST


#define buf_len 128


START_TEST(Complete_debug_note_renders_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();

    float expected_buf[buf_len] = { 0.0f };
    float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, seq);

    kqt_Handle_mix(handle, buf_len);
    check_unexpected_error();

    float* buf_l = kqt_Handle_get_buffer(handle, 0);
    float* buf_r = kqt_Handle_get_buffer(handle, 1);
    check_unexpected_error();

    check_buffers_equal(expected_buf, buf_l, buf_len, 0.0f);
    check_buffers_equal(expected_buf, buf_r, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_off_stops_the_note_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 20;

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_off_frame);

    // Note Off
    kqt_Handle_fire(handle, 0, "[\"n-\", null]");
    check_unexpected_error();
    mix_and_fill(actual_buf + note_off_frame, buf_len - note_off_frame);

    float expected_buf[buf_len] = { 0.0f };
    float seq_on[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    int offset = repeat_seq_local(expected_buf, 5, seq_on);
    float seq_off[] = { -1.0f, -0.5f, -0.5f, -0.5f };
    repeat_seq_local(expected_buf + offset, 2, seq_off);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_end_is_reached_correctly_during_note_off)
{
    set_mixing_rate(440);
    set_mix_volume(0);
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 70;

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_off_frame);

    // Note Off
    kqt_Handle_fire(handle, 0, "[\"n-\", null]");
    check_unexpected_error();
    mix_and_fill(actual_buf + note_off_frame, buf_len - note_off_frame);

    float expected_buf[buf_len] = { 0.0f };
    float seq_on[] = { 1.0f, 0.5f, 0.5f, 0.5f,
                       0.5f, 0.5f, 0.5f, 0.5f };
    int offset = repeat_seq_local(expected_buf, 8, seq_on);
    float seq_on_tail[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    offset += repeat_seq_local(expected_buf + offset, 1, seq_on_tail);
    float seq_off[] = { -0.5f, -0.5f, -1.0f, -0.5f, -0.5f, -0.5f,
                                      -0.5f, -0.5f, -0.5f, -0.5f };
    repeat_seq_local(expected_buf + offset, 1, seq_off);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Implicit_note_off_is_triggered_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf + note_2_frame, buf_len - note_2_frame);

    float expected_buf[buf_len] = { 0.0f };
    float seq_1_on[] = { 1.0f, 0.5f };
    int offset = repeat_seq_local(expected_buf, 1, seq_1_on);
    float seq_1_off[] = { 0.5f, 0.0f, -0.5f, 0.0f };
    offset += repeat_seq_local(expected_buf + offset, 2, seq_1_off);
    float seq_2[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf + offset, 8, seq_2);

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Independent_notes_mix_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    kqt_Handle_fire(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire(handle, 1, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf + note_2_frame, buf_len - note_2_frame);

    float expected_buf[buf_len] = { 0.0f };
    float single_seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, single_seq);
    for (int i = 40; i >= 0; --i)
    {
        expected_buf[i + note_2_frame] += expected_buf[i];
    }

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Debug_single_shot_renders_one_pulse)
{
    set_mix_volume(0);
    setup_debug_single_pulse();
    pause();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Empty_pattern_contains_silence)
{
    set_mixing_rate(mixing_rates[_i]);

    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [16, 0] }");

    const long expected_length = 8 * mixing_rates[_i];
    long actual_length = 0;

    long nframes = kqt_Handle_mix(handle, 4096);
    check_unexpected_error();
    while (nframes > 0)
    {
        actual_length += nframes;

        // Don't want to spend too much time on this...
        if (_i == MIXING_RATE_LOW)
        {
            float* bufs[] =
            {
                kqt_Handle_get_buffer(handle, 0),
                kqt_Handle_get_buffer(handle, 1),
            };
            check_unexpected_error();

            float expected_buf[128] = { 0.0f };
            check_buffers_equal(expected_buf, bufs[0], nframes, 0.0f);
            check_buffers_equal(expected_buf, bufs[1], nframes, 0.0f);
        }

        nframes = kqt_Handle_mix(handle, 4096);
        check_unexpected_error();
    }

    fail_unless(actual_length == expected_length,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", expected_length, actual_length));
}
END_TEST


START_TEST(Note_on_at_pattern_end_is_handled)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_def[128] = "";
    snprintf(pat0_def, sizeof(pat0_def), "{ \"length\": [%d, 0] }", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", pat0_def);

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 0], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_pattern.json", "{ \"length\": [8, 0] }");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i / 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_on_after_pattern_end_is_ignored)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_def[128] = "";
    snprintf(pat0_def, sizeof(pat0_def), "{ \"length\": [%d, 0] }", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", pat0_def);

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 1], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_pattern.json", "{ \"length\": [8, 0] }");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Initial_tempo_is_set_correctly)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    char ss_def[128] = "";
    snprintf(ss_def, sizeof(ss_def), "{ \"tempo\": %d }", tempos[_i]);
    set_data("song_00/p_song.json", ss_def);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]], [[1, 0], [\"n+\", \"0\"]] ]");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i]] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Infinite_mode_loops_composition)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [2, 0] }");
    set_data("pat_000/col_00/p_triggers.json", "[ [[0, 0], [\"n+\", \"0\"]] ]");

    kqt_Handle_fire(handle, 0, "[\"I.infinite\", true]");
    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    long mixed = mix_and_fill(actual_buf, buf_len);
    fail_unless(mixed == buf_len,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", buf_len, mixed));

    float expected_buf[buf_len] = { 0.0f };
    for (int i = 0; i < buf_len; i += mixing_rates[MIXING_RATE_LOW])
    {
        expected_buf[i] = 1.0f;
    }

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


Suite* Handle_suite(void)
{
    Suite* s = suite_create("Handle");

    int timeout = 4;

    TCase* tc_empty = tcase_create("empty");
    suite_add_tcase(s, tc_empty);
    tcase_set_timeout(tc_empty, timeout);
    tcase_add_checked_fixture(tc_empty, setup_empty, handle_teardown);

    tcase_add_test(tc_empty, Do_nothing);
    tcase_add_test(tc_empty, Initial_error_message_is_empty_string);
    tcase_add_loop_test(
            tc_empty, Empty_composition_has_zero_duration,
            0, SONG_SELECTION_COUNT);
    tcase_add_test(tc_empty, Default_mixing_rate_is_correct);
    tcase_add_test(tc_empty, Empty_composition_renders_zero_frames);
    tcase_add_loop_test(
            tc_empty, Set_mixing_rate,
            0, MIXING_RATE_COUNT);

    TCase* tc_render = tcase_create("render");
    suite_add_tcase(s, tc_render);
    tcase_set_timeout(tc_render, timeout);
    tcase_add_checked_fixture(tc_render, setup_empty, handle_teardown);
    tcase_add_checked_fixture(tc_render, setup_debug_instrument, NULL);

    tcase_add_test(tc_render, Do_nothing);
    tcase_add_loop_test(
            tc_render, Set_mixing_rate,
            0, MIXING_RATE_COUNT);

    // Note mixing
    tcase_add_test(tc_render, Complete_debug_note_renders_correctly);
    tcase_add_test(tc_render, Note_off_stops_the_note_correctly);
    tcase_add_test(tc_render, Note_end_is_reached_correctly_during_note_off);
    tcase_add_test(tc_render, Implicit_note_off_is_triggered_correctly);
    tcase_add_test(tc_render, Independent_notes_mix_correctly);
    tcase_add_test(tc_render, Debug_single_shot_renders_one_pulse);

    // Patterns
    tcase_add_loop_test(
            tc_render, Empty_pattern_contains_silence,
            0, MIXING_RATE_COUNT);
    tcase_add_loop_test(tc_render, Note_on_at_pattern_end_is_handled, 0, 4);
    tcase_add_loop_test(tc_render, Note_on_after_pattern_end_is_ignored, 0, 4);

    // Songs
    tcase_add_loop_test(tc_render, Initial_tempo_is_set_correctly, 0, 4);
    tcase_add_test(tc_render, Infinite_mode_loops_composition);

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


