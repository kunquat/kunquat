

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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


START_TEST(Maximum_relative_error_is_small)
{
    static const double small = 0.001;
    static const int32_t test_count = 1048577;

    for (int32_t i = 0; i < test_count; ++i)
    {
        const double x = (864.0 * i / test_count) - 432.0;
        const double result = fast_exp2(x);
        const double std_exp2 = exp2(x);
        const double rel_error = fabs((result / std_exp2) - 1);

        fail_unless(rel_error <= small,
                "fast_exp2(%.17g) yields %.17g, which is too far from %.17g",
                x, result, std_exp2);
    }
}
END_TEST


static Suite* Fast_exp2_suite(void)
{
    Suite* s = suite_create("Fast_exp2");

    static const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_correctness = tcase_create("correctness");
    suite_add_tcase(s, tc_correctness);
    tcase_set_timeout(tc_correctness, timeout);

    tcase_add_test(tc_correctness, Maximum_relative_error_is_small);

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


