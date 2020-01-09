

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Filter_utils.h>

#include <debug/assert.h>
#include <intrinsics.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fast_tan.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define MIN_CUTOFF_RATIO 0.00003
#define MAX_CUTOFF_RATIO 0.49


#define ENABLE_FILTER_SSE (KQT_SSE && KQT_SSE2 && KQT_SSE4_1)


#if ENABLE_FILTER_SSE

static __m128 get_cutoff_fast_f4(__m128 rel_freq)
{
    const __m128 pi = _mm_set1_ps((float)PI);
    const __m128 scaled_freq = _mm_mul_ps(pi, rel_freq);
    return fast_tan_pos_f4(scaled_freq);
}

#else

static float get_cutoff_fast(double rel_freq)
{
    dassert(rel_freq > 0);
    dassert(rel_freq < 0.5);

    const double scaled_freq = PI * rel_freq;

    return (float)fast_tan(scaled_freq);
}

#endif


static float get_cutoff(double rel_freq)
{
    rassert(rel_freq > 0);
    rassert(rel_freq < 0.5);
    return (float)tan(PI * rel_freq);
}


static float get_cutoff_ratio(double cutoff_param, int32_t audio_rate)
{
    const double clamped_cutoff_param = clamp(cutoff_param, -36, 136);
    const double cutoff_ratio =
        cents_to_Hz((clamped_cutoff_param - 24) * 100) / audio_rate;
    return (float)clamp(cutoff_ratio, MIN_CUTOFF_RATIO, MAX_CUTOFF_RATIO);
}


void transform_cutoff(
        Work_buffer* dest,
        const Work_buffer* src,
        double def_cutoff,
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(dest != NULL);
    rassert(isfinite(def_cutoff));
    rassert(frame_count > 0);
    rassert(audio_rate > 0);

    int32_t fast_cutoff_stop = 0;
    float const_cutoff = NAN;

    float* cutoffs = Work_buffer_get_contents_mut(dest);

    if (Work_buffer_is_valid(src))
    {
        const int32_t const_start = Work_buffer_get_const_start(src);
        fast_cutoff_stop = min(const_start, frame_count);
        const float* cutoff_buf = Work_buffer_get_contents(src);

        // Get cutoff values from input
#if ENABLE_FILTER_SSE
        const __m128 inv_audio_rate = _mm_set_ps1((float)(1.0 / audio_rate));
        for (int32_t i = 0; i < fast_cutoff_stop; i += 4)
        {
            const __m128 cutoff_param = _mm_load_ps(cutoff_buf + i);
            const __m128 offset = _mm_set_ps1(-24);
            const __m128 scale = _mm_set_ps1(100);
            const __m128 cutoff_ratio = _mm_mul_ps(
                    fast_cents_to_Hz_f4(
                        _mm_mul_ps(_mm_add_ps(cutoff_param, offset), scale)),
                    inv_audio_rate);

            const __m128 min_ratio = _mm_set_ps1((float)MIN_CUTOFF_RATIO);
            const __m128 max_ratio = _mm_set_ps1((float)MAX_CUTOFF_RATIO);
            const __m128 cutoff_ratio_clamped =
                _mm_min_ps(_mm_max_ps(min_ratio, cutoff_ratio), max_ratio);

            const __m128 cutoff = get_cutoff_fast_f4(cutoff_ratio_clamped);
            _mm_store_ps(cutoffs + i, cutoff);
        }
#else
        for (int32_t i = 0; i < fast_cutoff_stop; ++i)
        {
            const double cutoff_param = cutoff_buf[i];
            const double cutoff_ratio =
                fast_cents_to_Hz((cutoff_param - 24) * 100) / audio_rate;
            const double cutoff_ratio_clamped =
                clamp(cutoff_ratio, MIN_CUTOFF_RATIO, MAX_CUTOFF_RATIO);

            cutoffs[i] = get_cutoff_fast(cutoff_ratio_clamped);
        }
#endif

        if (fast_cutoff_stop < frame_count)
        {
            const double cutoff_ratio =
                get_cutoff_ratio(cutoff_buf[fast_cutoff_stop], audio_rate);
            const_cutoff = get_cutoff(cutoff_ratio);
        }
    }
    else
    {
        const double cutoff_ratio = get_cutoff_ratio(def_cutoff, audio_rate);
        const_cutoff = get_cutoff(cutoff_ratio);
    }

    for (int32_t i = fast_cutoff_stop; i < frame_count; ++i)
        cutoffs[i] = const_cutoff;

    Work_buffer_set_const_start(dest, fast_cutoff_stop);

    return;
}


