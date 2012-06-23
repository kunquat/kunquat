

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


#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


kqt_Handle* handle = NULL;


#define KT_VALUES(fmt, expected, actual) \
    "\n    Expected: " fmt \
    "\n    Actual:   " fmt , expected, actual


static void check_unexpected_error()
{
    char* error_string = kqt_Handle_get_error(handle);
    fail_unless(
            strcmp(error_string, "") == 0,
            "Unexpected error"
            KT_VALUES("%s", "", error_string));
    return;
}


typedef enum
{
    SUBSONG_SELECTION_FIRST,
    SUBSONG_SELECTION_LAST,
    SUBSONG_SELECTION_ALL,
    SUBSONG_SELECTION_COUNT
} Subsong_selection;


static int subsongs[] =
{
    [SUBSONG_SELECTION_FIRST] = 0,
    [SUBSONG_SELECTION_LAST]  = KQT_SUBSONGS_MAX - 1,
    [SUBSONG_SELECTION_ALL]   = -1,
};


typedef enum
{
    MIXING_RATE_LOW,
    MIXING_RATE_CD,
    MIXING_RATE_DEFAULT,
    MIXING_RATE_HIGH,
    MIXING_RATE_COUNT
} Mixing_rate;


static long mixing_rates[] =
{
    [MIXING_RATE_LOW]     = 8,
    [MIXING_RATE_CD]      = 44100,
    [MIXING_RATE_DEFAULT] = 48000,
    [MIXING_RATE_HIGH]    = 384000,
};


static void setup_empty(void)
{
    assert(handle == NULL);
    handle = kqt_new_Handle_m();
    fail_if(handle == NULL,
            "Couldn't create handle:\n%s\n", kqt_Handle_get_error(NULL));
    return;
}


static void set_data(char* key, char* data)
{
    assert(handle != NULL);
    assert(key != NULL);
    assert(data != NULL);

    kqt_Handle_set_data(handle, key, data, strlen(data) + 1);
    check_unexpected_error();
}


static void setup_debug_instrument(void)
{
    assert(handle != NULL);

    set_data("p_connections.json",
            "[ [\"ins_00/kqtiXX/out_00\", \"out_00\"] ]");

    set_data("ins_00/kqtiXX/p_connections.json",
            "[ [\"gen_00/kqtgXX/C/out_00\", \"out_00\"] ]");

    set_data("ins_00/kqtiXX/gen_00/kqtgXX/p_gen_type.json", "\"debug\"");

    return;
}


static void setup_debug_single_pulse(void)
{
    assert(handle != NULL);

    char* single = "true";
    kqt_Handle_set_data(handle,
            "ins_00/kqtiXX/gen_00/kqtgXX/c/p_single_pulse.jsonb",
            single, strlen(single) + 1);
    check_unexpected_error();

    return;
}


static void handle_teardown(void)
{
    assert(handle != NULL);
    kqt_del_Handle(handle);
    handle = NULL;
    return;
}


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
    long long dur = kqt_Handle_get_duration(handle, subsongs[_i]);
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


static void set_mixing_rate(long rate)
{
    assert(handle != NULL);
    assert(rate > 0);
    kqt_Handle_set_mixing_rate(handle, rate);
    check_unexpected_error();
    long actual_rate = kqt_Handle_get_mixing_rate(handle);
    check_unexpected_error();
    fail_unless(
            actual_rate == rate,
            "Wrong mixing rate"
            KT_VALUES("%ld", rate, actual_rate));
}


START_TEST(Set_mixing_rate)
{
    set_mixing_rate(mixing_rates[_i]);
}
END_TEST


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


static void fail_buffers(
        float* expected,
        float* actual,
        int offset,
        int len)
{
    const int margin = 4;
    const int start_idx = max(0, offset - margin);
    const int end_idx = min(len, offset + margin + 1);

    char indices[128] =       "\n             ";
    char expected_vals[128] = "\n    Expected:";
    char actual_vals[128] =   "\n      Actual:";
    int chars_used = strlen(indices);

    char* indices_ptr = indices + chars_used;
    char* expected_vals_ptr = expected_vals + chars_used;
    char* actual_vals_ptr = actual_vals + chars_used;

    ++chars_used;

    for (int i = start_idx; i < end_idx; ++i)
    {
        const int chars_left = 128 - chars_used;
        assert(chars_left > 0);

        int ilen = snprintf(indices_ptr, chars_left, " %5d", i);
        int elen = snprintf(expected_vals_ptr, chars_left,
                " %5.1f", expected[i]);
        int alen = snprintf(actual_vals_ptr, chars_left,
                " %5.1f", actual[i]);
        assert(ilen == elen);
        assert(ilen == alen);
        chars_used += ilen;

        indices_ptr += ilen;
        expected_vals_ptr += elen;
        actual_vals_ptr += alen;
    }

    fail("Buffers differ at offset %d:%s%s%s",
            offset, indices, expected_vals, actual_vals);
}


static void check_buffers_equal(
        float* expected,
        float* actual,
        int len,
        float eps)
{
    assert(expected != NULL);
    assert(actual != NULL);
    assert(len >= 0);
    assert(eps >= 0);

    for (int i = 0; i < len; ++i)
    {
        if (fabs(expected[i] - actual[i]) > eps)
        {
            fail_buffers(expected, actual, i, len);
            break;
        }
    }

    return;
}


static void set_mix_volume(double vol)
{
    assert(handle != NULL);

    char comp_def[] = "{ \"mix_vol\": -384.00000000 }";
    snprintf(comp_def, strlen(comp_def) + 1, "{ \"mix_vol\": %.4f }", vol);
    set_data("p_composition.json", comp_def);

    return;
}


static void pause(void)
{
    assert(handle != NULL);

    kqt_Handle_fire(handle, 0, "[\"Ipause\", null]");
    check_unexpected_error();

    return;
}


#define repeat_seq_local(dest, times, seq) \
    repeat_seq((dest), (times), sizeof((seq)) / sizeof(float), (seq))

int repeat_seq(float* dest, int times, int seq_len, float* seq)
{
    assert(dest != NULL);
    assert(times >= 0);
    assert(seq_len >= 0);
    assert(seq != NULL);

    for (int i = 0; i < times; ++i)
    {
        memcpy(dest, seq, seq_len * sizeof(float));
        dest += seq_len;
    }

    return times * seq_len;
}


#define buf_len 128


START_TEST(Complete_debug_note_renders_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    // 55 Hz
    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
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


static void mix_and_fill(float* buf, long nframes)
{
    assert(handle != NULL);
    assert(buf != NULL);
    assert(nframes >= 0);

    kqt_Handle_mix(handle, nframes);
    check_unexpected_error();
    float* ret_buf = kqt_Handle_get_buffer(handle, 0);
    check_unexpected_error();
    memcpy(buf, ret_buf, nframes * sizeof(float));
}


START_TEST(Note_off_stops_the_note_correctly)
{
    set_mixing_rate(220);
    set_mix_volume(0);
    pause();

    float actual_buf[buf_len] = { 0.0f };
    const int note_off_frame = 20;

    // 55 Hz
    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
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

    // 55 Hz
    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
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

    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
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

    kqt_Handle_fire(handle, 0, "[\"n+\", -3600]");
    check_unexpected_error();
    mix_and_fill(actual_buf, note_2_frame);

    kqt_Handle_fire(handle, 1, "[\"n+\", -3600]");
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

    set_data("subs_00/p_subsong.json", "{ \"patterns\": [0] }");
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


START_TEST(Note_on_is_handled_at_pattern_end)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    set_data("subs_00/p_subsong.json", "{ \"patterns\": [0, 1] }");

    char pat0_def[128] = "";
    snprintf(pat0_def, sizeof(pat0_def), "{ \"length\": [%d, 0] }", _i);
    set_data("pat_000/p_pattern.json", pat0_def);

    char col_def[128] = "";
    snprintf(col_def, sizeof(col_def), "[ [[%d, 0], [\"n+\", \"0\"]] ]", _i);
    set_data("pat_000/col_00/p_events.json", col_def);

    set_data("pat_001/p_pattern.json", "{ \"length\": [8, 0] }");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 0.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * _i / 2] = 1.0f;

    check_buffers_equal(expected_buf, actual_buf, buf_len, 0.0f);
}
END_TEST


START_TEST(Initial_tempo_is_set_correctly)
{
    set_mixing_rate(mixing_rates[MIXING_RATE_LOW]);
    set_mix_volume(0);
    setup_debug_single_pulse();

    int tempos[] = { 30, 60, 120, 240, 0 }; // 0 is guard, shouldn't be used

    char ss_def[128] = "";
    snprintf(ss_def, sizeof(ss_def),
            "{ \"tempo\": %d, \"patterns\": [0] }", tempos[_i]);
    set_data("subs_00/p_subsong.json", ss_def);
    set_data("pat_000/p_pattern.json", "{ \"length\": [4, 0] }");
    set_data("pat_000/col_00/p_events.json",
            "[ [[0, 0], [\"n+\", \"0\"]], [[1, 0], [\"n+\", \"0\"]] ]");

    float actual_buf[buf_len] = { 0.0f };
    mix_and_fill(actual_buf, buf_len);

    float expected_buf[buf_len] = { 1.0f };
    expected_buf[mixing_rates[MIXING_RATE_LOW] * 60 / tempos[_i]] = 1.0f;

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
            0, SUBSONG_SELECTION_COUNT);
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
    tcase_add_loop_test(tc_render, Note_on_is_handled_at_pattern_end, 0, 4);

    // Subsongs
    tcase_add_loop_test(tc_render, Initial_tempo_is_set_correctly, 0, 4);

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


