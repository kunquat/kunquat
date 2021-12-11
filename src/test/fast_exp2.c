

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2021
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <test_common.h>

#include <mathnum/fast_exp2.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>


#if defined(__GNUC__) && KQT_SSE4_1
#define ENABLE_F4_TEST 1
#else
#define ENABLE_F4_TEST 0
#endif


static double test_value(int32_t index, int32_t count)
{
    return (864.0 * index / count) - 432.0;
}


START_TEST(Maximum_relative_error_is_small)
{
    static const double small = 0.001;
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; ++i)
    {
        const double x = test_value(i, test_count);
        const double result = fast_exp2(x);
        const double std_exp2 = exp2(x);
        const double rel_error = fabs((result / std_exp2) - 1);

        ck_assert_msg(rel_error <= small,
                "fast_exp2(%.17g) yields %.17g, which is too far from %.17g",
                x, result, std_exp2);
    }
}
END_TEST


#if ENABLE_F4_TEST

static float test_value_f(int32_t index, int32_t count)
{
    return (float)(174.0 * index / count) - 87;
}

START_TEST(Maximum_relative_error_is_small_f4)
{
    static const float small = 0.002f;
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; i += 4)
    {
        const float x_data[4] __attribute__((aligned(16))) =
        {
            test_value_f(i, test_count),
            test_value_f(i + 1, test_count),
            test_value_f(i + 2, test_count),
            test_value_f(i + 3, test_count),
        };

        const __m128 x = _mm_load_ps(x_data);

        const __m128 result = fast_exp2_f4(x);
        float result_data[4] __attribute__((aligned(16)));
        _mm_store_ps(result_data, result);

        for (int k = 0; k < 4; ++k)
        {
            const float std_exp2f = exp2f(x_data[k]);
            const float rel_error = fabsf((result[k] / std_exp2f) - 1);

            ck_assert_msg(rel_error <= small,
                    "fast_exp2_f4(%.7g) yields %.7g, which is too far from %.7g",
                    x_data[k], result_data[k], std_exp2f);
        }
    }
}
END_TEST

#endif // ENABLE_F4_TEST


static Suite* Fast_exp2_suite(void)
{
    Suite* s = suite_create("Fast_exp2");

    static const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_correctness = tcase_create("correctness");
    suite_add_tcase(s, tc_correctness);
    tcase_set_timeout(tc_correctness, timeout);

    tcase_add_test(tc_correctness, Maximum_relative_error_is_small);
#if ENABLE_F4_TEST
    tcase_add_test(tc_correctness, Maximum_relative_error_is_small_f4);
#endif

    return s;
}


int main(void)
{
    Suite* suite = Fast_exp2_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    const int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


