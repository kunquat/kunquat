

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <test_common.h>

#include <mathnum/fast_log2.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>


#if defined(__GNUC__) && KQT_SSE2
#define ENABLE_F4_TEST 1
#else
#define ENABLE_F4_TEST 0
#endif


START_TEST(Maximum_absolute_error_is_small)
{
    static const double small = 0.001;
    static const int32_t test_count = 1048574;

    for (int32_t i = 1; i < test_count + 1; ++i)
    {
        const double x = (i / (double)test_count);
        const double result = fast_log2(x);
        const double std_log2 = log2(x);
        const double abs_error = fabs(result - std_log2);

        fail_unless(abs_error <= small,
                "fast_log2(%.17g) yields %.17g, which is %.17g from %.17g",
                x, result, abs_error, std_log2);
    }
}
END_TEST


#if ENABLE_F4_TEST
START_TEST(Maximum_absolute_error_is_small_f4)
{
    static const float small = 0.001f;
    static const int32_t test_count = 1048574;

    for (int32_t i = 1; i < test_count + 1; i += 4)
    {
        const float x_data[4] __attribute__((aligned(16))) =
        {
            (float)(i / (double)test_count),
            (float)((i + 1) / (double)test_count),
            (float)((i + 2) / (double)test_count),
            (float)((i + 3) / (double)test_count),
        };
        const __m128 x = _mm_load_ps(x_data);
        const __m128 result = fast_log2_f4(x);
        float result_data[4] __attribute__((aligned(16)));
        _mm_store_ps(result_data, result);

        for (int k = 0; k < 4; ++k)
        {
            const float std_log2 = log2f(x_data[k]);
            const float abs_error = fabsf(result_data[k] - std_log2);

            fail_unless(abs_error <= small,
                    "fast_log2_f4(%.7g) yields %.7g, which is %.7g from %.7g",
                    x_data[k], result_data[k], abs_error, std_log2);
        }
    }
}
END_TEST
#endif // ENABLE_F4_TEST


static Suite* Fast_log2_suite(void)
{
    Suite* s = suite_create("Fast_log2");

    static const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_correctness = tcase_create("correctness");
    suite_add_tcase(s, tc_correctness);
    tcase_set_timeout(tc_correctness, timeout);

    tcase_add_test(tc_correctness, Maximum_absolute_error_is_small);
#if ENABLE_F4_TEST
    tcase_add_test(tc_correctness, Maximum_absolute_error_is_small_f4);
#endif

    return s;
}


int main(void)
{
    Suite* suite = Fast_log2_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    const int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


