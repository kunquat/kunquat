

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
#include <player/Player.h>
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


#define buf_len 128


START_TEST(Complete_debug_note_renders_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    fail_if(
            !Player_set_audio_rate(player, 220),
            "Could not set player audio rate");

    setup_debug_instrument();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    float expected_buf[buf_len] = { 0.0f };
    const float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, seq);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", buf_len, nframes));

    check_unexpected_error();

    const float* buf_l = Player_get_audio(player, 0);
    const float* buf_r = Player_get_audio(player, 1);
    check_unexpected_error();

    check_buffers_equal(expected_buf, buf_l, buf_len, 0.0f);
    check_buffers_equal(expected_buf, buf_r, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_off_stops_the_note_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    fail_if(
            !Player_set_audio_rate(player, 220),
            "Could not set player audio rate");

    setup_debug_instrument();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    const int note_off_frame = 20;
    Player_play(player, note_off_frame);
    int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == note_off_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)note_off_frame, (long)nframes));

    float actual_buf[buf_len] = { 0.0f };
    const float* ret_buf = Player_get_audio(player, 0);
    for (int i = 0; i < note_off_frame; ++i)
        actual_buf[i] = ret_buf[i];

    check_unexpected_error();

    // Note Off
    if (!Player_fire(player, 0, "[\"n-\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    Player_play(player, buf_len - note_off_frame);
    ret_buf = Player_get_audio(player, 0);

    nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len - note_off_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)(buf_len - note_off_frame), (long)nframes));

    for (int i = 0; i < buf_len - note_off_frame; ++i)
        actual_buf[i + note_off_frame] = ret_buf[i];

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
    fail_if(
            !Player_set_audio_rate(player, 440),
            "Could not set player audio rate");

    setup_debug_instrument();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 70;

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();
    Player_play(player, note_off_frame);
    int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == note_off_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)note_off_frame, (long)nframes));

    const float* ret_buf = Player_get_audio(player, 0);
    for (int i = 0; i < note_off_frame; ++i)
        actual_buf[i] = ret_buf[i];

    check_unexpected_error();

    // Note Off
    if (!Player_fire(player, 0, "[\"n-\", null]", rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();

    Player_play(player, buf_len - note_off_frame);
    ret_buf = Player_get_audio(player, 0);

    nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len - note_off_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)(buf_len - note_off_frame), (long)nframes));

    for (int i = 0; i < buf_len - note_off_frame; ++i)
        actual_buf[i + note_off_frame] = ret_buf[i];

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
    fail_if(
            !Player_set_audio_rate(player, 220),
            "Could not set player audio rate");

    setup_debug_instrument();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();
    Player_play(player, note_2_frame);
    int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == note_2_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)note_2_frame, (long)nframes));

    const float* ret_buf = Player_get_audio(player, 0);
    for (int i = 0; i < note_2_frame; ++i)
        actual_buf[i] = ret_buf[i];

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();

    Player_play(player, buf_len - note_2_frame);
    ret_buf = Player_get_audio(player, 0);

    nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len - note_2_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)(buf_len - note_2_frame), (long)nframes));

    for (int i = 0; i < buf_len - note_2_frame; ++i)
        actual_buf[i + note_2_frame] = ret_buf[i];

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
    fail_if(
            !Player_set_audio_rate(player, 220),
            "Could not set player audio rate");

    setup_debug_instrument();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    if (!Player_fire(player, 0, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();
    Player_play(player, note_2_frame);
    int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == note_2_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)note_2_frame, (long)nframes));

    const float* ret_buf = Player_get_audio(player, 0);
    for (int i = 0; i < note_2_frame; ++i)
        actual_buf[i] = ret_buf[i];

    if (!Player_fire(player, 1, Note_On_55_Hz, rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();

    Player_play(player, buf_len - note_2_frame);
    ret_buf = Player_get_audio(player, 0);

    nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len - note_2_frame,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)(buf_len - note_2_frame), (long)nframes));

    for (int i = 0; i < buf_len - note_2_frame; ++i)
        actual_buf[i + note_2_frame] = ret_buf[i];

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

    setup_debug_instrument();
    setup_debug_single_pulse();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    if (!Player_fire(player, 0, "[\"n+\", 0]", rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);
    fail_unless(nframes == buf_len,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", (long)buf_len, (long)nframes));

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 1.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
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
            const float* bufs[] =
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


START_TEST(Note_on_at_pattern_end_is_handled)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_def[128] = "";
    snprintf(pat0_def, sizeof(pat0_def), "{ \"length\": [%d, 0] }", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", pat0_def);
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 0], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_pattern.json", "{ \"length\": [8, 0] }");
    set_data("pat_001/instance_000/p_manifest.json", "{}");

    validate();

    Player_reset(player);
    Player_play(player, 2048);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[2048] = { 0.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i / 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Note_on_after_pattern_end_is_ignored)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_def[128] = "";
    snprintf(pat0_def, sizeof(pat0_def), "{ \"length\": [%d, 0] }", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", pat0_def);
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 1], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_pattern.json", "{ \"length\": [8, 0] }");
    set_data("pat_001/instance_000/p_manifest.json", "{}");

    validate();

    Player_reset(player);
    Player_play(player, 2048);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[2048] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
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
            KT_VALUES("%ld", 0L, (long)nframes));
    fail_unless(
            Player_has_stopped(player),
            "Player did not reach end of composition");
}
END_TEST


START_TEST(Initial_tempo_is_set_correctly)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    char ss_def[128] = "";
    snprintf(ss_def, sizeof(ss_def), "{ \"tempo\": %d }", tempos[_i]);
    set_data("song_00/p_song.json", ss_def);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]], [[1, 0], [\"n+\", \"0\"]] ]");

    validate();

    Player_reset(player);
    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 1.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i]] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Infinite_mode_loops_composition)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [2, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json", "[ [[0, 0], [\"n+\", \"0\"]] ]");

    validate();

    Player_reset(player);

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"I.infinite\", true]", rs))
        fail("Could not fire event: %s", rs->message);
    check_unexpected_error();

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);
    fail_unless(nframes == buf_len,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", buf_len, nframes));

    float expected_buf[buf_len] = { 0.0f };
    for (int i = 0; i < buf_len; i += mixing_rates[MIXING_RATE_LOW])
    {
        expected_buf[i] = 1.0f;
    }

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Pattern_delay_extends_gap_between_trigger_rows)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[128] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"mpd\", \"%d\"]],"
            "  [[2, 0], [\"n+\", \"0\"]] ]", _i * 2);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    Player_reset(player);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    expected_buf[mixing_rates[MIXING_RATE_LOW] * (_i + 1)] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Pattern_delay_inserts_gap_between_adjacent_triggers)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[128] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[0, 0], [\"mpd\", \"%d\"]],"
            "  [[0, 0], [\"n+\", \"0\"]] ]", _i * 2);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    Player_reset(player);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i] += 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Tempo_change_affects_playback_cursor)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[256] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"m.t\", \"%d\"]],"
            "  [[2, 0], [\"n+\", \"0\"]] ]", tempos[_i]);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    Player_reset(player);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    const int second_offset = mixing_rates[MIXING_RATE_LOW] / 2;
    expected_buf[second_offset] = 1.0f;
    const int beat_len = mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i];
    expected_buf[second_offset + beat_len] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Tempo_slide_affects_playback_cursor)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[256] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"m/t\", \"%d\"]],"
            "  [[1, 0], [\"m/=t\", \"1\"]],"
            "  [[2, 0], [\"n+\", \"0\"]],"
            "  [[3, 0], [\"n+\", \"0\"]] ]", tempos[_i]);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    Player_reset(player);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    const int second_offset = mixing_rates[MIXING_RATE_LOW] / 2;
    expected_buf[second_offset] = 1.0f;

    int third_offset = 0;
    for (int i = second_offset + 1; i < buf_len; ++i)
    {
        if (actual_buf[i] == 1.0f)
        {
            third_offset = i;
            expected_buf[i] = 1.0f;
            break;
        }
    }
    fail_if(third_offset == 0, "Third pulse not found");

    if (tempos[_i] < 120)
    {
        fail_unless((third_offset - second_offset) >= second_offset,
                "Pulse interval was not increased during slide down");
    }
    else if (tempos[_i] > 120)
    {
        fail_unless((third_offset - second_offset) <= second_offset,
                "Pulse interval was not decreased during slide up");
    }
    else
    {
        fail_unless((third_offset - second_offset) == second_offset,
                "Pulse interval was changed without slide");
    }

    const int beat_len = mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i];
    expected_buf[third_offset + beat_len] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Jump_backwards_creates_a_loop)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    fail_if(
            !Player_set_audio_rate(player, mixing_rates[MIXING_RATE_LOW]),
            "Could not set player audio rate");
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[256] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[2, 0], [\"m.jc\", \"%d\"]],"
            "  [[2, 0], [\"mj\", null]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    Player_reset(player);

    Player_play(player, buf_len);
    const int32_t nframes = Player_get_frames_available(player);

    const float* actual_buf = Player_get_audio(player, 0);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    for (int i = 0; i < _i; ++i)
    {
        const int dist = mixing_rates[MIXING_RATE_LOW];
        expected_buf[dist + (i * dist)] = 1.0f;
    }

    check_buffers_equal(expected_buf, actual_buf, nframes, 0.0f);
}
END_TEST


START_TEST(Events_appear_in_event_buffer)
{
    setup_debug_instrument();
    setup_debug_single_pulse();

    check_unexpected_error();

    Player_reset(player);

    const char* actual_events = Player_get_events(player);
    const char expected_events_none[] = "[]";

    fail_unless(strcmp(actual_events, expected_events_none) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_none, actual_events));

    Read_state* rs = READ_STATE_AUTO;
    if (!Player_fire(player, 0, "[\"Ipause\", null]", rs))
        fail("Could not fire event: %s", rs->message);

    actual_events = Player_get_events(player);
    const char expected_events_1[] =
        "[[0, [\"Ipause\", null]]]";

    fail_unless(strcmp(actual_events, expected_events_1) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_1, actual_events));

    if (!Player_fire(player, 2, "[\".arpi\", 0]", rs))
        fail("Could not fire event: %s", rs->message);

    actual_events = Player_get_events(player);
    const char expected_events_2[] =
        "[[2, [\".arpi\", 0]]]";

    fail_unless(strcmp(actual_events, expected_events_2) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_2, actual_events));
}
END_TEST


Suite* Player_suite(void)
{
    Suite* s = suite_create("Player");

    const int timeout = 4;

#define BUILD_TCASE(name)                                                \
    TCase* tc_##name = tcase_create(#name);                              \
    suite_add_tcase(s, tc_##name);                                       \
    tcase_set_timeout(tc_##name, timeout);                               \
    tcase_add_checked_fixture(tc_##name, setup_player, player_teardown)

    BUILD_TCASE(general);
    BUILD_TCASE(notes);
    BUILD_TCASE(patterns);
    BUILD_TCASE(songs);
    BUILD_TCASE(events);

#undef BUILD_TCASE

    tcase_add_test(tc_general, Create_player);

    // Note mixing
    tcase_add_test(tc_notes, Complete_debug_note_renders_correctly);
    tcase_add_test(tc_notes, Note_off_stops_the_note_correctly);
    tcase_add_test(tc_notes, Note_end_is_reached_correctly_during_note_off);
    tcase_add_test(tc_notes, Implicit_note_off_is_triggered_correctly);
    tcase_add_test(tc_notes, Independent_notes_mix_correctly);
    tcase_add_test(tc_notes, Debug_single_shot_renders_one_pulse);

    // Patterns
    tcase_add_loop_test(
            tc_patterns, Empty_pattern_contains_silence,
            0, MIXING_RATE_COUNT);
    tcase_add_loop_test(tc_patterns, Note_on_at_pattern_end_is_handled, 0, 4);
    tcase_add_loop_test(tc_patterns, Note_on_after_pattern_end_is_ignored, 0, 4);

    // Songs
    tcase_add_test(tc_songs, Empty_composition_renders_zero_frames);
    tcase_add_loop_test(tc_songs, Initial_tempo_is_set_correctly, 0, 4);
    tcase_add_test(tc_songs, Infinite_mode_loops_composition);

    // Events
    tcase_add_loop_test(
            tc_events, Pattern_delay_extends_gap_between_trigger_rows,
            0, 4);
    tcase_add_loop_test(
            tc_events, Pattern_delay_inserts_gap_between_adjacent_triggers,
            0, 4);
    tcase_add_loop_test(
            tc_events, Tempo_change_affects_playback_cursor,
            0, 4);
    tcase_add_loop_test(
            tc_events, Tempo_slide_affects_playback_cursor,
            0, 4);
    tcase_add_loop_test(
            tc_events, Jump_backwards_creates_a_loop,
            0, 4);
    tcase_add_test(tc_events, Events_appear_in_event_buffer);

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


