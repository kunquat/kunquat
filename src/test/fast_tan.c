

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018-2021
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <test_common.h>

#include <mathnum/common.h>
#include <mathnum/fast_tan.h>

#include <stdint.h>
#include <stdio.h>


#if defined(__GNUC__) && KQT_SSE
#define ENABLE_F4_TEST 1
#else
#define ENABLE_F4_TEST 0
#endif


static float test_value_pos(int32_t index, int32_t count)
{
    return (float)(0.01 + (index / (double)count) * PI * 0.24);
}

START_TEST(Maximum_relative_error_is_small)
{
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; ++i)
    {
        const double small = lerp(0.00001f, 0.003f, i / (double)test_count);

        const double x = test_value_pos(i, test_count);
        const double result = fast_tan(x);
        const double std_tan = tan(x);
        const double rel_error = fabs((result / std_tan) - 1);

        ck_assert_msg(rel_error <= small,
                "fast_tan(%.7g) yields %.7g, which is too far from %.7g",
                x, result, std_tan);
    }
}
END_TEST

#if ENABLE_F4_TEST

START_TEST(Maximum_relative_error_is_small_f4)
{
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; i += 4)
    {
        const float small = lerp(0.00001f, 0.003f, (float)i / (float)test_count);

        const float x_data[4] __attribute__((aligned(16))) =
        {
            test_value_pos(i, test_count),
            test_value_pos(i + 1, test_count),
            test_value_pos(i + 2, test_count),
            test_value_pos(i + 3, test_count),
        };

        const __m128 x = _mm_load_ps(x_data);
        const __m128 result = fast_tan_pos_f4(x);
        float result_data[4] __attribute__((aligned(16)));
        _mm_store_ps(result_data, result);

        for (int k = 0; k < 4; ++k)
        {
            const float std_tan = tanf(x_data[k]);
            const float rel_error = fabsf((result[k] / std_tan) - 1);

            ck_assert_msg(rel_error <= small,
                    "fast_tan_pos_f4(%.7g) yields %.7g, which is too far from %.7g",
                    x_data[k], result_data[k], std_tan);
        }
    }
}
END_TEST

#endif


static Suite* Fast_tan_suite(void)
{
    Suite* s = suite_create("Fast_tan");

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
    Suite* suite = Fast_tan_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    const int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


