

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2013-2016
 *          Ossi Saresoja, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <test_common.h>

#include <kunquat/testing.h>
#include <mathnum/common.h>
#include <player/devices/processors/Filter.h>

#include <math.h>
#include <string.h>


#define sample_count (1 << 16)

#define cutoff_count 15

#define default_q (1.0 / sqrt(2.0))

#define lowpass 0
//#define highpass 1


static double fwarp(double f)
{
    if (f == -0.5)
        return -INFINITY;
    else if (f == 0.5)
        return INFINITY;

    return 2 * tan(PI * f);
}


static double lpass2lpass(double f, double f1)
{
    return f / f1;
}


static double two_pole_power(double f, double q)
{
    const double ff = f * f;
    const double iq = 1 / q;
    return 1 / (1 + ff * (ff + iq*iq - 2));
}


START_TEST(Two_Pole_Frequency_Response)
{
    const double cutoff_freq = ((float)_i / (float)(cutoff_count + 1)) * 0.5;

    double coeffs[2] = { 0.0 };
    double mul = 0.0;
    two_pole_filter_create(cutoff_freq, default_q, lowpass, coeffs, &mul);

    double history_cos_1[2] = { 0.0 };
    double history_cos_2[2] = { 0.0 };
    double history_sin_1[2] = { 0.0 };
    double history_sin_2[2] = { 0.0 };

    const int filter_order = 2;

    const double wave_freq = 0.25;

    double fcos = 0.0;
    double fsin = 0.0;

    for (int i = 0; i < sample_count; ++i)
    {
        const double x = i * (2 * PI * wave_freq);

        const double cos_val = cos(x);
        fcos = nq_zero_filter(filter_order, history_cos_1, cos_val);
        fcos = iir_filter_strict_cascade(filter_order, coeffs, history_cos_2, fcos);
        fcos *= mul;

        const double sin_val = sin(x);
        fsin = nq_zero_filter(filter_order, history_sin_1, sin_val);
        fsin = iir_filter_strict_cascade(filter_order, coeffs, history_sin_2, fsin);
        fsin *= mul;
    }

    const double actual_power = (fcos * fcos) + (fsin * fsin);

    const double expected_power = two_pole_power(
            lpass2lpass(fwarp(wave_freq), fwarp(cutoff_freq)),
            default_q);

    const double tolerance = 0.001; // FIXME: proper value based on params

    fail_unless(fabs(actual_power - expected_power) <= tolerance,
            "\nExpected power: %.6f\n  Actual power: %.6f",
            expected_power, actual_power);
}
END_TEST


static Suite* Filter_suite(void)
{
    Suite* s = suite_create("Filter");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_two_pole = tcase_create("two_pole");
    suite_add_tcase(s, tc_two_pole);
    tcase_set_timeout(tc_two_pole, timeout);
    //tcase_add_checked_fixture(tc_two_pole, setup_empty, handle_teardown);

    tcase_add_loop_test(
            tc_two_pole, Two_Pole_Frequency_Response,
            1, cutoff_count + 1);

    return s;
}


int main(void)
{
    Suite* suite = Filter_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


