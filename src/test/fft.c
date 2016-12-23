

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

#include <mathnum/common.h>
#include <mathnum/fft.h>
#include <mathnum/Random.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define max_test_length 256
static FFT_worker* fw = FFT_WORKER_AUTO;


void setup_fft_worker(void)
{
    FFT_worker* tfw = FFT_worker_init(fw, max_test_length);
    fail_if(tfw == NULL, "Could not allocate memory for FFT worker.");
    return;
}


void fft_worker_teardown(void)
{
    FFT_worker_deinit(fw);
    return;
}


START_TEST(Factorisation_is_valid)
{
    float data[max_test_length] = { 0 };

    const int check_val = _i;

    FFT_worker_rfft(fw, data, check_val);

    fail_unless(fw->ifac[0] == check_val,
            "Calculated length (%d) does not match the given length (%d)",
            (int)fw->ifac[0], check_val);
    fail_unless(fw->ifac[1] > 0,
            "Number of factors (%d) is not positive", fw->ifac[1]);

    int32_t prod = 1;
    for (int i = 0; i < fw->ifac[1]; ++i)
        prod *= fw->ifac[2 + i];

    fail_unless(prod == check_val,
            "Product of calculated factors (%d) does not equal given length (%d)",
            prod, check_val);
}
END_TEST


typedef void (*fill_func)(float*, int);

static void fill_data_base_freq(float* data, int length)
{
    for (int i = 0; i < length; ++i)
        data[i] = (float)cos(i * 2 * PI / (float)length);

    return;
}

static void fill_data_saw(float* data, int length)
{
    for (int i = 0; i < length; ++i)
        data[i] = (float)i / (float)length;

    return;
}

static void fill_data_noise(float* data, int length)
{
    Random* random = Random_init(RANDOM_AUTO, "test");

    for (int i = 0; i < length; ++i)
        data[i] = (float)Random_get_float_signal(random);

    return;
}


START_TEST(Forward_and_inverse_transform_return_scaled_original)
{
    const int test_length = _i;

    float orig_data[max_test_length] = { 0 };
    float data[max_test_length] = { 0 };

    fill_func funcs[] =
    {
        fill_data_base_freq,
        fill_data_saw,
        fill_data_noise,
    };
    const size_t funcs_count = sizeof(funcs) / sizeof(*funcs);

    for (size_t fi = 0; fi < funcs_count; ++fi)
    {
        funcs[fi](orig_data, test_length);
        memcpy(data, orig_data, sizeof(data));

        FFT_worker_rfft(fw, data, test_length);
        FFT_worker_irfft(fw, data, test_length);

        for (int i = 0; i < test_length; ++i)
            data[i] /= (float)test_length;

        for (int i = 0; i < test_length; ++i)
        {
            fail_if(fabsf(data[i] - orig_data[i]) > 0.001f,
                    "Absolute value is too large at index %d:"
                    " converted %.7g, original %.7g",
                    i, data[i], orig_data[i]);
        }
    }
}
END_TEST


static Suite* FFT_suite(void)
{
    Suite* s = suite_create("FFT");

    static const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_correctness = tcase_create("correctness");
    suite_add_tcase(s, tc_correctness);
    tcase_set_timeout(tc_correctness, timeout);
    tcase_add_checked_fixture(tc_correctness, setup_fft_worker, fft_worker_teardown);

    tcase_add_loop_test(tc_correctness, Factorisation_is_valid, 2, max_test_length + 1);
    tcase_add_loop_test(
            tc_correctness,
            Forward_and_inverse_transform_return_scaled_original,
            1,
            max_test_length + 1);

    return s;
}


int main(void)
{
    Suite* suite = FFT_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    const int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


