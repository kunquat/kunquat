

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
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
#include <string/Streader.h>

#include <stdint.h>
#include <string.h>


#define buf_len 128


START_TEST(Complete_debug_note_renders_correctly)
{
    set_audio_rate(220);
    set_mix_volume(0);
    setup_debug_instrument();
    pause();

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();

    float expected_buf[buf_len] = { 0.0f };
    const float seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, seq);

    kqt_Handle_play(handle, buf_len);
    check_unexpected_error();

    const float* buf_l = kqt_Handle_get_audio(handle, 0);
    const float* buf_r = kqt_Handle_get_audio(handle, 1);
    check_unexpected_error();

    check_buffers_equal(expected_buf, buf_l, buf_len, 0.0f);
    check_buffers_equal(expected_buf, buf_r, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_off_stops_the_note_correctly)
{
    set_audio_rate(220);
    set_mix_volume(0);
    setup_debug_instrument();
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 20;

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_off_frame);

    // Note Off
    kqt_Handle_fire_event(handle, 0, "[\"n-\", null]");
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
    set_audio_rate(440);
    set_mix_volume(0);
    setup_debug_instrument();
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 70;

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_off_frame);

    // Note Off
    kqt_Handle_fire_event(handle, 0, "[\"n-\", null]");
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
    set_audio_rate(220);
    set_mix_volume(0);
    setup_debug_instrument();
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
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
    set_audio_rate(220);
    set_mix_volume(0);
    setup_debug_instrument();
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_2_frame = 2;

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire_event(handle, 1, Note_On_55_Hz);
    check_unexpected_error();
    mix_and_fill(actual_buf + note_2_frame, buf_len - note_2_frame);

    float expected_buf[buf_len] = { 0.0f };
    float single_seq[] = { 1.0f, 0.5f, 0.5f, 0.5f };
    repeat_seq_local(expected_buf, 10, single_seq);
    for (int i = 40; i >= 0; --i)
        expected_buf[i + note_2_frame] += expected_buf[i];

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Debug_single_shot_renders_one_pulse)
{
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();
    pause();

    float actual_buf[buf_len] = { 0.0f };

    kqt_Handle_fire_event(handle, 0, "[\"n+\", 0]");
    check_unexpected_error();
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Empty_pattern_contains_silence)
{
    set_audio_rate(mixing_rates[_i]);
    setup_debug_instrument();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[16, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    validate();

    const long expected_length = 8 * mixing_rates[_i];
    long actual_length = 0;

    kqt_Handle_play(handle, 4096);
    check_unexpected_error();
    while (!kqt_Handle_has_stopped(handle))
    {
        const long nframes = kqt_Handle_get_frames_available(handle);
        check_unexpected_error();
        actual_length += nframes;

        // Don't want to spend too much time on this...
        if (_i == MIXING_RATE_LOW)
        {
            const float* bufs[] =
            {
                kqt_Handle_get_audio(handle, 0),
                kqt_Handle_get_audio(handle, 1),
            };
            check_unexpected_error();

            float expected_buf[128] = { 0.0f };
            assert(nframes <= 128);
            check_buffers_equal(expected_buf, bufs[0], nframes, 0.0f);
            check_buffers_equal(expected_buf, bufs[1], nframes, 0.0f);
        }

        kqt_Handle_play(handle, 4096);
        check_unexpected_error();
    }

    const long nframes = kqt_Handle_get_frames_available(handle);
    check_unexpected_error();
    actual_length += nframes;

    fail_unless(actual_length == expected_length,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", expected_length, actual_length));
}
END_TEST


START_TEST(Note_on_at_pattern_end_is_handled)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_length[128] = "";
    snprintf(pat0_length, sizeof(pat0_length), "[%d, 0]", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", pat0_length);
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 0], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_length.json", "[8, 0]");
    set_data("pat_001/instance_000/p_manifest.json", "{}");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i / 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_on_after_pattern_end_is_ignored)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    char pat0_length[128] = "";
    snprintf(pat0_length, sizeof(pat0_length), "[%d, 0]", _i);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", pat0_length);
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 1], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", col_def);

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_length.json", "[8, 0]");
    set_data("pat_001/instance_000/p_manifest.json", "{}");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Note_on_at_pattern_start_is_handled)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [1, 0] ]");

    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]] ]");

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_length.json", "[8, 0]");
    set_data("pat_001/instance_000/p_manifest.json", "{}");
    set_data("pat_001/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]] ]");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    expected_buf[mixing_rates[MIXING_RATE_LOW] * 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Pattern_playback_repeats_pattern)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [1, 0], [0, 0], [1, 1] ]");

    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[1, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json", "[ [[0, 0], [\"n+\", \"0\"]] ]");

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_length.json", "[16, 0]");
    set_data("pat_001/instance_000/p_manifest.json", "{}");
    set_data("pat_001/instance_001/p_manifest.json", "{}");

    validate();

    kqt_Handle_fire_event(handle, 0, "[\"cpattern\", [0, 0]]");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    for (long i = 0; i < buf_len; i += mixing_rates[MIXING_RATE_LOW] / 2)
        expected_buf[i] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Pattern_playback_pauses_zero_length_pattern)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [1, 0], [0, 0], [1, 1] ]");

    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[0, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json", "[ [[0, 0], [\"n+\", \"0\"]] ]");

    set_data("pat_001/p_manifest.json", "{}");
    set_data("pat_001/p_length.json", "[16, 0]");
    set_data("pat_001/instance_000/p_manifest.json", "{}");
    set_data("pat_001/instance_001/p_manifest.json", "{}");
    set_data("pat_001/col_01/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]], [[1, 0], [\"n+\", \"0\"]] ]");

    validate();

    kqt_Handle_fire_event(handle, 0, "[\"cpattern\", [0, 0]]");

    float actual_buf[buf_len] = { 0.0f };
    long frames_left = buf_len;
    long buf_offset = 0;
    for (int i = 0; i < 100 && frames_left > 0; ++i)
    {
        kqt_Handle_play(handle, frames_left);
        check_unexpected_error();
        const long frames_available = kqt_Handle_get_frames_available(handle);
        const float* ret_buf = kqt_Handle_get_audio(handle, 0);
        check_unexpected_error();
        memcpy(actual_buf + buf_offset,
                ret_buf,
                (size_t)frames_available * sizeof(float));

        buf_offset += frames_available;
        frames_left -= frames_available;
    }

    fail_if(frames_left == buf_len,
            "Pattern playback of zero-length pattern produces no audio");

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Empty_composition_renders_zero_frames)
{
    kqt_Handle_play(handle, 256);
    check_unexpected_error();

    const long nframes = kqt_Handle_get_frames_available(handle);

    fail_unless(
            nframes == 0,
            "Wrong number of frames rendered"
            KT_VALUES("%ld", 0L, nframes));
}
END_TEST


START_TEST(Paused_empty_composition_contains_silence)
{
    pause();

    kqt_Handle_play(handle, buf_len);
    check_unexpected_error();

    const long frames_available = kqt_Handle_get_frames_available(handle);
    fail_if(frames_available != buf_len,
            "kqt_Handle_play rendered %ld instead of %d frames",
            frames_available, buf_len);

    const float* ret_bufs[] =
    {
        kqt_Handle_get_audio(handle, 0),
        kqt_Handle_get_audio(handle, 1),
    };

    float expected_buf[buf_len] = { 0.0f };

    check_buffers_equal(expected_buf, ret_bufs[0], buf_len, 0.0f);
    check_buffers_equal(expected_buf, ret_bufs[1], buf_len, 0.0f);
}
END_TEST


START_TEST(Initial_tempo_is_set_correctly)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    char tempo_data[128] = "";
    snprintf(tempo_data, sizeof(tempo_data), "%d", tempos[_i]);
    set_data("song_00/p_tempo.json", tempo_data);
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]], [[1, 0], [\"n+\", \"0\"]] ]");

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i]] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Infinite_mode_loops_composition)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[2, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json", "[ [[0, 0], [\"n+\", \"0\"]] ]");

    validate();

    kqt_Handle_fire_event(handle, 0, "[\"cinfinite+\", null]");
    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    const long mixed = mix_and_fill(actual_buf, buf_len);
    fail_unless(mixed == buf_len,
            "Wrong number of frames mixed"
            KT_VALUES("%ld", buf_len, mixed));

    float expected_buf[buf_len] = { 0.0f };
    for (long i = 0; i < buf_len; i += mixing_rates[MIXING_RATE_LOW])
        expected_buf[i] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Skipping_moves_position_forwards)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[8, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"n+\", \"0\"]],"
            "  [[2, 0], [\"n+\", \"0\"]],"
            "  [[3, 0], [\"n+\", \"0\"]],"
            "  [[4, 0], [\"n+\", \"0\"]] ]");

    validate();

    static const long long second = 1000000000LL;
    kqt_Handle_set_position(handle, 0, _i * second / 2);
    //Player_skip(player, _i * mixing_rates[MIXING_RATE_LOW] / 2);
    check_unexpected_error();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    for (int i = 0; i < 5 - _i; ++i)
        expected_buf[i * mixing_rates[MIXING_RATE_LOW] / 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Pattern_delay_extends_gap_between_trigger_rows)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[128] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"mpd\", \"%d\"]],"
            "  [[2, 0], [\"n+\", \"0\"]] ]", _i * 2);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    expected_buf[mixing_rates[MIXING_RATE_LOW] * (_i + 1)] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Pattern_delay_inserts_gap_between_adjacent_triggers)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[128] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[0, 0], [\"mpd\", \"%d\"]],"
            "  [[0, 0], [\"n+\", \"0\"]] ]", _i * 2);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i] += 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Tempo_change_affects_playback_cursor)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[256] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"n+\", \"0\"]],"
            "  [[1, 0], [\"m.t\", \"%d\"]],"
            "  [[2, 0], [\"n+\", \"0\"]] ]", tempos[_i]);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    const long second_offset = mixing_rates[MIXING_RATE_LOW] / 2;
    expected_buf[second_offset] = 1.0f;
    const long beat_len = mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i];
    expected_buf[second_offset + beat_len] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Tempo_slide_affects_playback_cursor)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
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

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    const long second_offset = mixing_rates[MIXING_RATE_LOW] / 2;
    expected_buf[second_offset] = 1.0f;

    long third_offset = 0;
    for (long i = second_offset + 1; i < buf_len; ++i)
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

    const long beat_len = mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i];
    expected_buf[third_offset + beat_len] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Jump_backwards_creates_a_loop)
{
    set_audio_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    char triggers[256] = "";
    snprintf(triggers, sizeof(triggers),
            "[ [[0, 0], [\"n+\", \"0\"]],"
            "  [[2, 0], [\"m.jc\", \"%d\"]],"
            "  [[2, 0], [\"mj\", null]] ]", _i);
    set_data("pat_000/col_00/p_triggers.json", triggers);

    validate();

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[0] = 1.0f;
    for (int i = 0; i < _i; ++i)
    {
        const long dist = mixing_rates[MIXING_RATE_LOW];
        expected_buf[dist + (i * dist)] = 1.0f;
    }

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Events_appear_in_event_buffer)
{
    setup_debug_instrument();
    setup_debug_single_pulse();

    const char* actual_events = kqt_Handle_receive_events(handle);
    check_unexpected_error();
    const char expected_events_none[] = "[]";

    fail_unless(strcmp(actual_events, expected_events_none) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_none, actual_events));

    kqt_Handle_fire_event(handle, 0, "[\"cpause\", null]");
    check_unexpected_error();

    actual_events = kqt_Handle_receive_events(handle);
    check_unexpected_error();
    const char expected_events_1[] =
        "[[0, [\"cpause\", null]]]";

    fail_unless(strcmp(actual_events, expected_events_1) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_1, actual_events));

    kqt_Handle_fire_event(handle, 2, "[\".arpi\", 0]");
    check_unexpected_error();

    actual_events = kqt_Handle_receive_events(handle);
    check_unexpected_error();
    const char expected_events_2[] =
        "[[2, [\".arpi\", 0]]]";

    fail_unless(strcmp(actual_events, expected_events_2) == 0,
            "Wrong events received"
            KT_VALUES("%s", expected_events_2, actual_events));
}
END_TEST


void setup_many_triggers(int event_count)
{
    // Set up pattern essentials
    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    // Add lots of simultaneous triggers
    char* triggers = malloc(sizeof(char) * 65536);
    fail_if(triggers == NULL, "Could not allocate memory for triggers");

    char* cur_pos = triggers;
    cur_pos += sprintf(triggers, "[ [[0, 0], [\"n+\", \"0\"]]");
    for (int i = 1; i < event_count; ++i)
    {
        assert(cur_pos - triggers < 65000);
        cur_pos += snprintf(
                cur_pos,
                (size_t)(65536 - (cur_pos - triggers)),
                ", [[0, 0], [\"%s\", \"%d\"]]",
                (i % 16 == 0) ? "n+" : "vs",
                i);
    }

    cur_pos += snprintf(cur_pos, (size_t)(65536 - (cur_pos - triggers)), " ]");

    set_data("pat_000/col_00/p_triggers.json", triggers);
    free(triggers);
    triggers = NULL;

    validate();

    return;
}


bool read_received_events(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    (void)index;
    assert(userdata != NULL);

    int32_t* expected = userdata;
    double actual = NAN;

    const char* event_name = (*expected % 16 == 0) ? "n+" : "vs";

    if (!(Streader_readf(sr, "[0, [") &&
                Streader_match_string(sr, event_name) &&
                Streader_readf(sr, ", %f]]", &actual))
       )
        return false;

    if ((int)round(actual) != *expected)
    {
        Streader_set_error(
                sr,
                "Received argument %" PRId64 " instead of %" PRId32,
                actual, *expected);
        return false;
    }

    *expected = (int)round(actual) + 1;

    return true;
}


START_TEST(Events_from_many_triggers_can_be_retrieved_with_multiple_receives)
{
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    const int event_count = 2049;
    const int note_count = event_count / 16 + 1;
    fail_if(note_count >= 512, "Too many notes to check correct audio output");

    setup_many_triggers(event_count);

    // Play
    kqt_Handle_play(handle, 10);
    const long frames_available = kqt_Handle_get_frames_available(handle);
    fail_if(frames_available > 0,
            "Kunquat handle rendered %ld frames of audio"
            " although event buffer was filled",
            frames_available);

    // Receive and make sure all events are found
    const char* events = kqt_Handle_receive_events(handle);
    int32_t expected = 0;
    int loop_count = 0;
    while (strcmp("[]", events) != 0)
    {
        Streader* sr = Streader_init(STREADER_AUTO, events, (int64_t)strlen(events));
        fail_if(!Streader_read_list(sr, read_received_events, &expected),
                "Event list reading failed: %s",
                Streader_get_error_desc(sr));

        events = kqt_Handle_receive_events(handle);
        ++loop_count;
    }

    fail_if(loop_count <= 1,
            "Test did not fill the event buffer, increase event count!");

    fail_if(expected != event_count,
            "Read %" PRId32 " instead of %d events",
            expected, event_count);

    // Continue playing
    kqt_Handle_play(handle, 10);
    fail_if(kqt_Handle_get_frames_available(handle) != 10,
            "Kunquat handle rendered %ld instead of 10 frames",
            kqt_Handle_get_frames_available(handle));

    const float expected_buf[10] = { (float)note_count };
    const float* actual_buf = kqt_Handle_get_audio(handle, 0);
    check_buffers_equal(expected_buf, actual_buf, 10, 0.0f);
}
END_TEST


START_TEST(Events_from_many_triggers_are_skipped_by_fire)
{
    const int event_count = 2048;

    setup_many_triggers(event_count);

    kqt_Handle_play(handle, 10);

    kqt_Handle_fire_event(handle, 0, "[\".a\", 0]");
    check_unexpected_error();

    const char* events = kqt_Handle_receive_events(handle);
    const char* expected = "[[0, [\".a\", 0]]]";
    fail_if(strcmp(events, expected) != 0,
            "Received event list %s instead of %s",
            events, expected);
}
END_TEST


void setup_complex_bind(int event_count)
{
    char* bind = malloc(sizeof(char) * 65536);
    fail_if(bind == NULL, "Could not allocate memory for bind specification");

    char* cur_pos = bind;
    cur_pos += sprintf(bind, "[[\"#\", [], [[0, [\"n+\", \"0\"]]");
    for (int i = 1; i < event_count; ++i)
    {
        assert(cur_pos - bind < 65000);
        cur_pos += snprintf(cur_pos, (size_t)(65536 - (cur_pos - bind)),
                ", [0, [\"n+\", \"%d\"]]", i);
    }

    cur_pos += snprintf(cur_pos, (size_t)(65536 - (cur_pos - bind)), " ]]]");

    set_data("p_bind.json", bind);
    free(bind);
    bind = NULL;

    validate();

    return;
}


void setup_complex_bind_trigger(void)
{
    // Set up pattern essentials
    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/col_00/p_triggers.json",
            "[ [[0, 0], [\"#\", \"\\\"\\\"\"]] ]");

    validate();

    return;
}


bool read_received_events_bind(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    (void)index;
    assert(userdata != NULL);

    int32_t* expected = userdata;
    double actual = NAN;

    if (index == 0 && *expected == 0)
    {
        return Streader_readf(sr, "[0, [") &&
            Streader_match_string(sr, "#") &&
            Streader_match_char(sr, ',') &&
            Streader_read_string(sr, 0, NULL) &&
            Streader_readf(sr, "]]");
    }

    if (!(Streader_readf(sr, "[0, [") &&
                Streader_match_string(sr, "n+") &&
                Streader_readf(sr, ", %f]]", &actual))
       )
        return false;

    if ((int)round(actual) != *expected)
    {
        Streader_set_error(
                sr,
                "Received argument %.0f instead of %" PRId32,
                actual, *expected);
        return false;
    }

    *expected = (int)round(actual) + 1;

    return true;
}


START_TEST(Events_from_complex_bind_can_be_retrieved_with_multiple_receives)
{
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    const int event_count = 2048;
    setup_complex_bind(event_count);
    setup_complex_bind_trigger();

    // Play
    kqt_Handle_play(handle, 10);
    const long frames_available = kqt_Handle_get_frames_available(handle);
    fail_if(frames_available > 0,
            "Kunquat handle rendered %ld frames of audio"
            " although event buffer was filled",
            frames_available);

    // Receive and make sure all events are found
    const char* events = kqt_Handle_receive_events(handle);
    int32_t expected = 0;
    int loop_count = 0;
    while (strcmp("[]", events) != 0)
    {
        Streader* sr = Streader_init(STREADER_AUTO, events, (int64_t)strlen(events));
        fail_if(!Streader_read_list(sr, read_received_events_bind, &expected),
                "Event list reading failed: %s",
                Streader_get_error_desc(sr));

        events = kqt_Handle_receive_events(handle);
        ++loop_count;
    }

    fail_if(loop_count <= 1,
            "Test did not fill the event buffer, increase event count!");

    fail_if(expected != event_count,
            "Read %" PRId32 " instead of %d events",
            expected, event_count);

    // Continue playing
    kqt_Handle_play(handle, 10);
    fail_if(kqt_Handle_get_frames_available(handle) != 10,
            "Kunquat handle rendered %ld instead of 10 frames",
            kqt_Handle_get_frames_available(handle));

    // FIXME: We can only check for 512 notes as we run out of voices :-P
    const float expected_buf[10] = { min((float)event_count, 512) };
    const float* actual_buf = kqt_Handle_get_audio(handle, 0);
    check_buffers_equal(expected_buf, actual_buf, 10, 0.0f);
}
END_TEST


START_TEST(Fire_with_complex_bind_can_be_processed_with_multiple_receives)
{
    set_mix_volume(0);
    setup_debug_instrument();
    setup_debug_single_pulse();

    const int event_count = 2048;
    setup_complex_bind(event_count);

    pause();
    kqt_Handle_fire_event(handle, 0, "[\"#\", \"\"]");
    check_unexpected_error();

    // Receive and make sure all events are found
    const char* events = kqt_Handle_receive_events(handle);
    int32_t expected = 0;
    int loop_count = 0;
    while (strcmp("[]", events) != 0)
    {
        Streader* sr = Streader_init(STREADER_AUTO, events, (int64_t)strlen(events));
        fail_if(!Streader_read_list(sr, read_received_events_bind, &expected),
                "Event list reading failed: %s",
                Streader_get_error_desc(sr));

        events = kqt_Handle_receive_events(handle);
        ++loop_count;
    }

    fail_if(loop_count <= 1,
            "Test did not fill the event buffer, increase event count!");

    fail_if(expected != event_count,
            "Read %" PRId32 " instead of %d events",
            expected, event_count);

    // Continue playing
    kqt_Handle_play(handle, 10);
    fail_if(kqt_Handle_get_frames_available(handle) != 10,
            "Kunquat handle rendered %ld instead of 10 frames",
            kqt_Handle_get_frames_available(handle));

    // FIXME: We can only check for 512 notes as we run out of voices :-P
    const float expected_buf[10] = { min((float)event_count, 512) };
    const float* actual_buf = kqt_Handle_get_audio(handle, 0);
    check_buffers_equal(expected_buf, actual_buf, 10, 0.0f);
}
END_TEST


static void setup_query_patterns(void)
{
    // Set up two empty pattern instances
    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0], [0, 1] ]");

    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_length.json", "[4, 0]");
    set_data("pat_000/instance_000/p_manifest.json", "{}");
    set_data("pat_000/instance_001/p_manifest.json", "{}");

    validate();

    return;
}


START_TEST(Query_initial_location)
{
    set_audio_rate(220);
    setup_query_patterns();

    // We need to seek or play before the current pattern is resolved
    kqt_Handle_set_position(handle, 0, 0);

    kqt_Handle_fire_event(handle, 0, "[\"qlocation\", null]");

    const char* events = kqt_Handle_receive_events(handle);
    const char* expected =
        "[[0, [\"qlocation\", null]]"
        ", [0, [\"Atrack\", 0]], [0, [\"Asystem\", 0]]"
        ", [0, [\"Apattern\", [0, 0]]], [0, [\"Arow\", [0, 0]]]"
        "]";

    fail_if(strcmp(events, expected) != 0,
            "Received event list %s instead of %s", events, expected);
}
END_TEST


START_TEST(Query_final_location)
{
    set_audio_rate(220);
    setup_query_patterns();

    kqt_Handle_play(handle, 2048);
    kqt_Handle_fire_event(handle, 0, "[\"qlocation\", null]");

    const char* events = kqt_Handle_receive_events(handle);
    const char* expected =
        "[[0, [\"qlocation\", null]]"
        ", [0, [\"Atrack\", 0]], [0, [\"Asystem\", 1]]"
        ", [0, [\"Apattern\", [0, 1]]], [0, [\"Arow\", [4, 0]]]"
        "]";

    fail_if(strcmp(events, expected) != 0,
            "Received event list %s instead of %s", events, expected);
}
END_TEST


START_TEST(Query_voice_count_with_silence)
{
    set_audio_rate(220);
    pause();

    kqt_Handle_play(handle, 128);
    kqt_Handle_fire_event(handle, 0, "[\"qvoices\", null]");

    const char* events = kqt_Handle_receive_events(handle);
    const char* expected =
        "[[0, [\"qvoices\", null]], [0, [\"Avoices\", 0]], [0, [\"Avgroups\", 0]]]";

    fail_if(strcmp(events, expected) != 0,
            "Received event list %s instead of %s", events, expected);
}
END_TEST


START_TEST(Query_voice_count_with_note)
{
    set_audio_rate(220);
    setup_debug_instrument();
    pause();

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);
    kqt_Handle_play(handle, 1);
    kqt_Handle_fire_event(handle, 0, "[\"qvoices\", null]");

    const char* events2 = kqt_Handle_receive_events(handle);
    const char* expected2 =
        "[[0, [\"qvoices\", null]], [0, [\"Avoices\", 2]], [0, [\"Avgroups\", 1]]]";

    fail_if(strcmp(events2, expected2) != 0,
            "Received event list %s instead of %s", events2, expected2);

    kqt_Handle_play(handle, 2048);
    kqt_Handle_fire_event(handle, 0, "[\"qvoices\", null]");

    const char* events1 = kqt_Handle_receive_events(handle);
    const char* expected1 =
        "[[0, [\"qvoices\", null]], [0, [\"Avoices\", 0]], [0, [\"Avgroups\", 0]]]";

    fail_if(strcmp(events1, expected1) != 0,
            "Received event list %s instead of %s", events1, expected1);

    /*
    kqt_Handle_play(handle, 1);
    kqt_Handle_fire_event(handle, 0, "[\"qvoices\", null]");

    const char* events0 = kqt_Handle_receive_events(handle);
    const char* expected0 = "[[0, [\"qvoices\", null]], [0, [\"Avoices\", 0]]]";

    fail_if(strcmp(events0, expected0) != 0,
            "Received event list %s instead of %s", events0, expected0);
    // */
}
END_TEST


static bool test_reported_force(Streader* sr, double expected)
{
    assert(sr != NULL);

    double actual = NAN;

    if (!(Streader_readf(sr, "[[0, [") &&
                Streader_match_string(sr, "qf") &&
                Streader_readf(sr, ", null]], [0, [") &&
                Streader_match_string(sr, "Af") &&
                Streader_readf(sr, ", %f]]]", &actual)))
        return false;

    if (fabs(actual - expected) > 0.1)
    {
        Streader_set_error(
                sr, "Expected force %.4f, got %.4f", expected, actual);
        return false;
    }

    return true;
}


START_TEST(Query_note_force)
{
    setup_debug_instrument();
    pause();

    kqt_Handle_fire_event(handle, 0, Note_On_55_Hz);

    kqt_Handle_fire_event(handle, 0, "[\"qf\", null]");
    const char* events0 = kqt_Handle_receive_events(handle);
    Streader* sr0 = Streader_init(STREADER_AUTO, events0, (int64_t)strlen(events0));
    fail_if(!test_reported_force(sr0, 0.0), Streader_get_error_desc(sr0));

    kqt_Handle_fire_event(handle, 0, "[\".f\", -6]");
    kqt_Handle_fire_event(handle, 0, "[\"qf\", null]");
    const char* events6 = kqt_Handle_receive_events(handle);
    Streader* sr6 = Streader_init(STREADER_AUTO, events6, (int64_t)strlen(events6));
    fail_if(!test_reported_force(sr6, -6.0), Streader_get_error_desc(sr6));
}
END_TEST


static Suite* Player_suite(void)
{
    Suite* s = suite_create("Player");

    const int timeout = DEFAULT_TIMEOUT;

#define BUILD_TCASE(name)                                                \
    TCase* tc_##name = tcase_create(#name);                              \
    suite_add_tcase(s, tc_##name);                                       \
    tcase_set_timeout(tc_##name, timeout);                               \
    tcase_add_checked_fixture(tc_##name, setup_empty, handle_teardown)

    //BUILD_TCASE(general);
    BUILD_TCASE(notes);
    BUILD_TCASE(patterns);
    BUILD_TCASE(songs);
    BUILD_TCASE(events);

#undef BUILD_TCASE

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
    tcase_add_test(tc_patterns, Note_on_at_pattern_start_is_handled);
    tcase_add_test(tc_patterns, Pattern_playback_repeats_pattern);
    tcase_add_test(tc_patterns, Pattern_playback_pauses_zero_length_pattern);

    // Songs
    tcase_add_test(tc_songs, Empty_composition_renders_zero_frames);
    tcase_add_test(tc_songs, Paused_empty_composition_contains_silence);
    tcase_add_loop_test(tc_songs, Initial_tempo_is_set_correctly, 0, 4);
    tcase_add_test(tc_songs, Infinite_mode_loops_composition);
    tcase_add_loop_test(tc_songs, Skipping_moves_position_forwards, 0, 4);

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
    tcase_add_test(
            tc_events,
            Events_from_many_triggers_can_be_retrieved_with_multiple_receives);
    tcase_add_test(
            tc_events,
            Events_from_many_triggers_are_skipped_by_fire);
    tcase_add_test(
            tc_events,
            Events_from_complex_bind_can_be_retrieved_with_multiple_receives);
    tcase_add_test(
            tc_events,
            Fire_with_complex_bind_can_be_processed_with_multiple_receives);
    tcase_add_test(tc_events, Query_initial_location);
    tcase_add_test(tc_events, Query_final_location);
    tcase_add_test(tc_events, Query_voice_count_with_silence);
    tcase_add_test(tc_events, Query_voice_count_with_note);
    tcase_add_test(tc_events, Query_note_force);

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


