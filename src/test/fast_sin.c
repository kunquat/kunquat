

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2021
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
#include <mathnum/fast_sin.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>


START_TEST(Sine_values_have_maximum_magnitude_of_one)
{
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; ++i)
    {
        const double x = 2.0 * PI * (double)i / (double)(test_count - 1);
        const double result = fast_sin(x);

        ck_assert_msg(result >= -1.0,
                "fast_sin(2 * pi * %" PRId32 " / %" PRId32 ") = fast_sin(%.17f)"
                " yields %.17f < -1.0",
                i, (test_count - 1), x, result);
        ck_assert_msg(result <= 1.0,
                "fast_sin(2 * pi * %" PRId32 " / %" PRId32 ") = fast_sin(%.17f)"
                " yields %.17f > 1.0",
                i, (test_count - 1), x, result);
    }
}
END_TEST


START_TEST(Maximum_absolute_error_is_small)
{
    static const double small = 0.0011;
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; ++i)
    {
        const double x = 2.0 * PI * (double)i / (double)(test_count - 1);
        const double result = fast_sin(x);
        const double std_sin = sin(x);
        const double abs_error = fabs(result - std_sin);

        ck_assert_msg(abs_error <= small,
                "fast_sin(2 * pi * %" PRId32 " / %" PRId32 ") = fast_sin(%.17f)"
                " yields %.17f, which is %.17f from %.17f",
                i, (test_count - 1), x, result, abs_error, std_sin);
    }
}
END_TEST


static Suite* Fast_sin_suite(void)
{
    Suite* s = suite_create("Fast_sin");

    static const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_correctness = tcase_create("correctness");
    suite_add_tcase(s, tc_correctness);
    tcase_set_timeout(tc_correctness, timeout);

    tcase_add_test(tc_correctness, Sine_values_have_maximum_magnitude_of_one);
    tcase_add_test(tc_correctness, Maximum_absolute_error_is_small);

    return s;
}


int main(void)
{
    Suite* suite = Fast_sin_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    const int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


