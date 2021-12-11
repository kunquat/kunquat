

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2021
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_HANDLE_UTILS_H
#define KQT_HANDLE_UTILS_H


#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static kqt_Handle handle = 0;


typedef enum
{
    SONG_SELECTION_FIRST,
    SONG_SELECTION_LAST,
    SONG_SELECTION_ALL,
    SONG_SELECTION_COUNT
} Subsong_selection;


static const int songs[] =
{
    [SONG_SELECTION_FIRST] = 0,
    [SONG_SELECTION_LAST]  = KQT_SONGS_MAX - 1,
    [SONG_SELECTION_ALL]   = -1,
};


typedef enum
{
    MIXING_RATE_LOW,
    MIXING_RATE_CD,
    MIXING_RATE_DEFAULT,
    MIXING_RATE_HIGH,
    MIXING_RATE_COUNT
} Mixing_rate;


static const long mixing_rates[] =
{
    [MIXING_RATE_LOW]     = 8,
    [MIXING_RATE_CD]      = 44100,
    [MIXING_RATE_DEFAULT] = 48000,
    [MIXING_RATE_HIGH]    = 384000,
};


#define Note_On_55_Hz "[\"n+\", -3600]"


void check_unexpected_error(void)
{
    const char* error_string = kqt_Handle_get_error(handle);
    ck_assert_msg(
            strcmp(error_string, "") == 0,
            "Unexpected error"
            KT_VALUES("%s", "", error_string));
    return;
}


void setup_empty(void)
{
    assert(handle == 0);
    handle = kqt_new_Handle();
    ck_assert_msg(handle != 0,
            "Couldn't create handle:\n%s\n", kqt_Handle_get_error(0));
    return;
}


void handle_teardown(void)
{
    assert(handle != 0);
    kqt_del_Handle(handle);
    handle = 0;
    return;
}


void set_data(const char* key, const char* data)
{
    assert(handle != 0);
    assert(key != NULL);
    assert(data != NULL);

    kqt_Handle_set_data(handle, key, data, (long)strlen(data));
    check_unexpected_error();
}


void validate(void)
{
    assert(handle != 0);

    kqt_Handle_validate(handle);
    check_unexpected_error();
}


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


void fail_buffers(
        const float* expected,
        const float* actual,
        long offset,
        long len)
{
    static const long margin = 4;
    const long start_idx = max(0, offset - margin);
    const long end_idx = min(len, offset + margin + 1);

    char indices[128] =       "\n             ";
    char expected_vals[128] = "\n    Expected:";
    char actual_vals[128] =   "\n      Actual:";
    int chars_used = (int)strlen(indices);

    char* indices_ptr = indices + chars_used;
    char* expected_vals_ptr = expected_vals + chars_used;
    char* actual_vals_ptr = actual_vals + chars_used;

    ++chars_used;

    for (long i = start_idx; i < end_idx; ++i)
    {
        const int chars_left = 128 - chars_used;
        assert(chars_left > 0);

        int ilen = snprintf(indices_ptr, (size_t)chars_left, " %5ld", i);
        int elen = snprintf(expected_vals_ptr, (size_t)chars_left,
                " %5.1f", expected[i]);
        int alen = snprintf(actual_vals_ptr, (size_t)chars_left,
                " %5.1f", actual[i]);
        //assert(ilen == elen);
        //assert(ilen == alen);
        chars_used += ilen;

        indices_ptr += ilen;
        expected_vals_ptr += elen;
        actual_vals_ptr += alen;
    }

    ck_abort_msg("Buffers differ at offset %ld:%s%s%s",
            offset, indices, expected_vals, actual_vals);
}


void check_buffers_equal(
        const float* expected,
        const float* actual,
        long len,
        float eps)
{
    assert(expected != NULL);
    assert(actual != NULL);
    assert(len >= 0);
    assert(eps >= 0);

    for (long i = 0; i < len; ++i)
    {
        if (!(fabs(expected[i] - actual[i]) <= eps))
        {
            fail_buffers(expected, actual, i, len);
            break;
        }
    }

    return;
}


void set_audio_rate(long rate)
{
    assert(handle != 0);
    assert(rate > 0);
    kqt_Handle_set_audio_rate(handle, rate);
    check_unexpected_error();
    long actual_rate = kqt_Handle_get_audio_rate(handle);
    check_unexpected_error();
    ck_assert_msg(
            actual_rate == rate,
            "Wrong audio rate"
            KT_VALUES("%ld", rate, actual_rate));
}


void set_mix_volume(double vol)
{
    assert(handle != 0);

    char mix_vol_def[128] = "[0, -384.00000000]";
    snprintf(mix_vol_def, strlen(mix_vol_def) + 1, "[0, %.4f]", vol);
    set_data("p_mixing_volume.json", mix_vol_def);
    validate();

    return;
}


void pause(void)
{
    assert(handle != 0);

    kqt_Handle_fire_event(handle, 0, "[\"cpause\", null]");
    check_unexpected_error();

    return;
}


#define repeat_seq_local(dest, times, seq) \
    repeat_seq((dest), (times), sizeof((seq)) / sizeof(float), (seq))

int repeat_seq(float* dest, int times, int seq_len, const float* seq)
{
    assert(dest != NULL);
    assert(times >= 0);
    assert(seq_len >= 0);
    assert(seq != NULL);

    for (int i = 0; i < times; ++i)
    {
        memcpy(dest, seq, (size_t)seq_len * sizeof(float));
        dest += seq_len;
    }

    return times * seq_len;
}


long mix_and_fill(float* buf, long nframes)
{
    assert(handle != 0);
    assert(buf != NULL);
    assert(nframes >= 0);

    kqt_Handle_play(handle, nframes);
    check_unexpected_error();
    const long frames_available = kqt_Handle_get_frames_available(handle);
    const float* ret_buf = kqt_Handle_get_audio(handle);
    check_unexpected_error();
    for (long i = 0; i < frames_available; ++i)
        buf[i] = ret_buf[i * 2];

    return frames_available;
}


void setup_debug_instrument(void)
{
    assert(handle != 0);

    set_data("p_dc_blocker_enabled.json", "[0, false]");

    set_data("out_00/p_manifest.json", "[0, {}]");
    set_data("out_01/p_manifest.json", "[0, {}]");
    set_data("p_connections.json",
            "[0,"
            "[ [\"au_00/out_00\", \"out_00\"]"
            ", [\"au_00/out_01\", \"out_01\"]"
            "]"
            "]");

    set_data("p_control_map.json", "[0, [[0, 0]]]");
    set_data("control_00/p_manifest.json", "[0, {}]");

    set_data("au_00/p_manifest.json", "[0, { \"type\": \"instrument\" }]");
    set_data("au_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/out_01/p_manifest.json", "[0, {}]");
    set_data("au_00/p_connections.json",
            "[0,"
            "[ [\"proc_00/C/out_00\", \"out_00\"]"
            ", [\"proc_00/C/out_01\", \"out_01\"]"
            ", [\"proc_01/C/out_00\", \"proc_00/C/in_00\"]"
            "]"
            "]");

    set_data("au_00/proc_00/p_manifest.json", "[0, { \"type\": \"debug\" }]");
    set_data("au_00/proc_00/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_00/proc_00/in_00/p_manifest.json", "[0, {}]");
    set_data("au_00/proc_00/out_00/p_manifest.json", "[0, {}]");
    set_data("au_00/proc_00/out_01/p_manifest.json", "[0, {}]");

    set_data("au_00/proc_01/p_manifest.json", "[0, { \"type\": \"pitch\" }]");
    set_data("au_00/proc_01/p_signal_type.json", "[0, \"voice\"]");
    set_data("au_00/proc_01/out_00/p_manifest.json", "[0, {}]");

    validate();

    check_unexpected_error();

    return;
}


void setup_debug_single_pulse(void)
{
    assert(handle != 0);

    set_data("p_dc_blocker_enabled.json", "[0, false]");

    set_data("au_00/proc_00/c/p_b_single_pulse.json", "[0, true]");

    validate();

    check_unexpected_error();

    return;
}


#endif // KQT_HANDLE_UTILS_H


