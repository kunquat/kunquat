

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <test_common.h>

#include <Streader.h>


#define init_with_cstr(s) Streader_init(STREADER_AUTO, (s), strlen((s)))

#define arr_size(arr) (sizeof(arr) / sizeof(*(arr)))


START_TEST(Initial_streader_has_no_error_set)
{
    const Streader* sr = init_with_cstr("");
    fail_if(Streader_is_error_set(sr),
            "Streader initialised with an empty string has an error set");

    sr = init_with_cstr("]]]]");
    fail_if(Streader_is_error_set(sr),
            "Streader initialised with an invalid string has an error set"
            " (before reading)");
}
END_TEST


START_TEST(Matching_visible_characters_succeed)
{
    static const char* ones[] =
    {
        "1",
        " 1",
        " 1 ",
    };
    for (size_t i = 0; i < arr_size(ones); ++i)
    {
        Streader* sr = init_with_cstr(ones[i]);

        fail_if(!Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", ones[i]);
        fail_if(Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", ones[i]);
        fail_if(Streader_match_char(sr, '1'),
                "Streader did not move forwards after a successful match"
                " in \"%s\"",
                ones[i]);
    }

    static const char* exprs[] =
    {
        "1+2",
        " 1+2",
        "1 +2",
        " 1 +2",
        "1+ 2",
        "1 + 2",
        " 1 + 2 ",
    };
    for (size_t i = 0; i < arr_size(exprs); ++i)
    {
        Streader* sr = init_with_cstr(exprs[i]);

        fail_if(!Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", exprs[i]);
        fail_if(Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
        fail_if(!Streader_match_char(sr, '+'),
                "Could not match '+' in \"%s\"", exprs[i]);
        fail_if(Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
        fail_if(!Streader_match_char(sr, '2'),
                "Could not match '2' in \"%s\"", exprs[i]);
        fail_if(Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
    }
}
END_TEST


START_TEST(Matching_wrong_characters_fails)
{
    Streader* sr = init_with_cstr("1");

    fail_if(Streader_match_char(sr, '2'),
            "Matched '2' successfully in \"1\"");
    fail_if(!Streader_is_error_set(sr),
            "No error set after a failed match");
    fail_if(Streader_match_char(sr, '1'),
            "Match succeeded after an error");

    Streader_clear_error(sr);
    fail_if(Streader_is_error_set(sr),
            "Streader_clear_error did not remove Streader error");
    fail_if(!Streader_match_char(sr, '1'),
            "Correct match did not succeed after a cleared failure");
}
END_TEST


START_TEST(Characters_past_specified_length_are_ignored)
{
    static struct
    {
        const char* str;
        int len;
    }
    nums[] =
    {
        { "123456", 3 },
        { "123 456", 4 },
        { "12 3456", 4 },
    };

    for (size_t i = 0; i < arr_size(nums); ++i)
    {
        Streader* sr = Streader_init(STREADER_AUTO, nums[i].str, nums[i].len);

        fail_if(!Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", nums[i].str);
        fail_if(!Streader_match_char(sr, '2'),
                "Could not match '2' in \"%s\"", nums[i].str);
        fail_if(!Streader_match_char(sr, '3'),
                "Could not match '3' in \"%s\"", nums[i].str);

        fail_if(Streader_match_char(sr, '4'),
                "Matched '4' located in \"%s\" after given length %d",
                nums[i].str,
                nums[i].len);
    }
}
END_TEST


START_TEST(Matching_strings_requires_quotes_in_data)
{
    Streader* sr = init_with_cstr("abc");
    fail_if(Streader_match_string(sr, "abc"),
            "Matched a string without double quotes in data");

    sr = init_with_cstr("abc\"");
    fail_if(Streader_match_string(sr, "abc"),
            "Matched a string without opening double quote in data");

    sr = init_with_cstr("\"abc");
    fail_if(Streader_match_string(sr, "abc"),
            "Matched a string without closing double quote in data");
}
END_TEST


START_TEST(Matching_strings_succeeds)
{
    Streader* sr = init_with_cstr("\"\"");
    fail_if(!Streader_match_string(sr, ""), "Could not match empty string");

    sr = init_with_cstr("\" \"");
    fail_if(!Streader_match_string(sr, " "),
            "Could not match a string with whitespace");

    sr = init_with_cstr("\"abc\"");
    fail_if(!Streader_match_string(sr, "abc"),
            "Could not match the string \"abc\"");

    sr = init_with_cstr("\"\" \"\"");
    fail_if(!Streader_match_string(sr, ""),
            "Could not match the first of two empty strings");
    fail_if(!Streader_match_string(sr, ""),
            "Could not match the second of two empty strings");
    fail_if(Streader_match_string(sr, ""),
            "Matched an empty string when end of data should have been reached");
}
END_TEST


START_TEST(Matching_wrong_strings_fails)
{
    Streader* sr = init_with_cstr("\"\"");
    fail_if(Streader_match_string(sr, " "),
            "Empty string and string with whitespace were matched");

    sr = init_with_cstr("\" \"");
    fail_if(Streader_match_string(sr, ""),
            "Empty string and string with whitespace were matched");
}
END_TEST


START_TEST(Reading_null_consumes_data)
{
    Streader* sr = init_with_cstr("null x");
    fail_if(!Streader_read_null(sr), "Could not read a null value");
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume the null value");
}
END_TEST


START_TEST(Null_token_with_trailing_garbage_is_rejected)
{
    Streader* sr = init_with_cstr("nullz");
    fail_if(Streader_read_null(sr),
            "Reading null token did not check for trailing garbage");
}
END_TEST


START_TEST(Reading_bool_stores_correct_value)
{
    Streader* sr = init_with_cstr("false x");
    bool result = true;
    fail_if(!Streader_read_bool(sr, &result), "Could not read a false value");
    fail_if(result != false,
            "Reading false stored %d", (int)result);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume the false value");

    sr = init_with_cstr("true x");
    result = false;
    fail_if(!Streader_read_bool(sr, &result), "Could not read a true value");
    fail_if(result != true,
            "Reading true stored %d", (int)result);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume the true value");
}
END_TEST


START_TEST(Bool_with_trailing_garbage_is_rejected)
{
    Streader* sr = init_with_cstr("falsez");
    bool result = false;
    fail_if(Streader_read_bool(sr, &result),
            "Streader accepted falsez as a Boolean false value");

    sr = init_with_cstr("truez");
    fail_if(Streader_read_bool(sr, &result),
            "Streader accepted truez as a Boolean true value");
}
END_TEST


START_TEST(Read_zero_int)
{
    Streader* sr = init_with_cstr("0 x");
    int64_t num = -1;
    fail_if(!Streader_read_int(sr, &num), "Could not read 0");
    fail_if(num != 0, "Streader stored %" PRId64 " instead of 0", num);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume 0 correctly");

    sr = init_with_cstr("-0 x");
    num = 1;
    fail_if(!Streader_read_int(sr, &num), "Could not read -0");
    fail_if(num != 0, "Streader stored %" PRId64 " instead of 0", num);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume -0 correctly");
}
END_TEST


START_TEST(Read_nonzero_int)
{
    char data[128] = "";

    static const int64_t nums[] = { 1, 19, INT64_MAX, -1, -19, INT64_MIN };

    for (size_t i = 0; i < arr_size(nums); ++i)
    {
        sprintf(data, "%" PRId64 " x", nums[i]);
        Streader* sr = init_with_cstr(data);
        int64_t num = 0;
        fail_if(!Streader_read_int(sr, &num),
                "Could not read %" PRId64,
                nums[i]);
        fail_if(num != nums[i],
                "Streader stored %" PRId64 " instead of %" PRId64,
                num, nums[i]);
        fail_if(!Streader_match_char(sr, 'x'),
                "Streader did not consume %" PRId64 " correctly",
                nums[i]);
    }
}
END_TEST


START_TEST(Reading_too_large_int_in_magnitude_fails)
{
    char data[4][128] = { "" };
    sprintf(data[0], "%" PRId64, INT64_MAX);
    sprintf(data[1], "%" PRId64, INT64_MIN);
    sprintf(data[2], "%" PRId64, INT64_MAX);
    sprintf(data[3], "%" PRId64, INT64_MIN);

    // Make sure the overflowing code below makes sense
    for (int i = 0; i < 4; ++i)
    {
        size_t len = strlen(data[i]);
        assert(len > 2);

        assert(isdigit(data[i][len - 1]));
        assert(data[i][len - 1] != '9');
        assert(isdigit(data[i][len - 2]));
        assert(data[i][len - 2] != '9');
    }

    // Overflow data[0] and data[1] by 1
    data[0][strlen(data[0]) - 1] += 1;
    data[1][strlen(data[1]) - 1] += 1;

    // Overflow data[2] and data[3] by 10
    data[2][strlen(data[2]) - 2] += 1;
    data[3][strlen(data[3]) - 2] += 1;

    // Test reading
    for (int i = 0; i < 4; ++i)
    {
        Streader* sr = init_with_cstr(data[i]);
        int64_t num = 0;
        fail_if(Streader_read_int(sr, &num),
                "Reading overflowing integer %s succeeded",
                data[i]);
    }
}
END_TEST


START_TEST(Read_zero_float)
{
    const char* zeros[] =
    {
        "0 x",
        "0.0 x",
        "0e0 x",
        "0.0e0 x",
        "0.0e+0 x",
        "0.0e-0 x",
    };

    for (size_t i = 0; i < arr_size(zeros); ++i)
    {
        Streader* sr = init_with_cstr(zeros[i]);
        double num = NAN;
        fail_if(!Streader_read_float(sr, &num),
                "Could not read 0 from \"%s\": %s",
                zeros[i], Streader_get_error_desc(sr));
        fail_if(num != 0,
                "Streader stored %f instead of 0 from \"%s\"",
                num, zeros[i]);
        fail_if(!Streader_match_char(sr, 'x'),
                "Streader did not consume 0 from \"%s\" correctly: %s",
                zeros[i], Streader_get_error_desc(sr));
    }

    // TODO: The code below does not test the sign of negative zero
    //       as C99 doesn't guarantee it.
    //       Revisit when migrating to emulated floats.

    const char* neg_zeros[] =
    {
        "-0 x",
        "-0.0 x",
        "-0e0 x",
        "-0.0e0 x",
        "-0.0e+0 x",
        "-0.0e-0 x",
    };

    for (size_t i = 0; i < arr_size(neg_zeros); ++i)
    {
        Streader* sr = init_with_cstr(neg_zeros[i]);
        double num = NAN;
        fail_if(!Streader_read_float(sr, &num),
                "Could not read -0 from \"%s\": %s",
                neg_zeros[i], Streader_get_error_desc(sr));
        fail_if(num != 0,
                "Streader stored %f instead of -0 from \"%s\"",
                num, neg_zeros[i]);
        fail_if(!Streader_match_char(sr, 'x'),
                "Streader did not consume -0 from \"%s\" correctly",
                neg_zeros[i]);
    }
}
END_TEST


START_TEST(Read_nonzero_float)
{
    const double nums[] = { 0.5, 1.0, 1.5, -0.5, -1.0, -1.5, };
    const char* formats[] =
    {
        "%.1f x",
        "%f x",
        "%e x",
    };

    for (size_t i = 0; i < arr_size(nums); ++i)
    {
        for (size_t k = 0; k < arr_size(formats); ++k)
        {
            char data[128] = "";
            sprintf(data, formats[k], nums[i]);

            Streader* sr = init_with_cstr(data);
            double num = NAN;
            fail_if(!Streader_read_float(sr, &num),
                    "Could not read float from \"%s\": %s",
                    data, Streader_get_error_desc(sr));
            fail_if(num != nums[i],
                    "Streader stored %f instead of %.2f from \"%s\"",
                    num, nums[i]);
            fail_if(!Streader_match_char(sr, 'x'),
                    "Streader did not consume float from \"%s\" correctly",
                    data);
        }
    }
}
END_TEST


START_TEST(Whitespace_terminates_decimal_number)
{
    Streader* sr = init_with_cstr("- 1");
    double num = NAN;
    fail_if(Streader_read_float(sr, &num),
            "Streader accepted \"- 1\" as a float");

    sr = init_with_cstr("-1 .5");
    num = NAN;
    fail_if(!Streader_read_float(sr, &num),
            "Could not read float from \"-1 .5\": %s",
            Streader_get_error_desc(sr));
    fail_if(num != -1, "Streader read %f instead of -1 from \"-1 .5\"", num);

    sr = init_with_cstr("-1. 5");
    num = NAN;
    fail_if(Streader_read_float(sr, &num),
            "Streader accepted \"-1.\" as a float");

    sr = init_with_cstr("-1 e5");
    num = NAN;
    fail_if(!Streader_read_float(sr, &num),
            "Could not read float from \"-1 e5\": %s",
            Streader_get_error_desc(sr));
    fail_if(num != -1, "Streader read %f instead of -1 from \"-1 e5\"", num);

    sr = init_with_cstr("-1e 5");
    num = NAN;
    fail_if(Streader_read_float(sr, &num),
            "Streader accepted \"-1e\" as a float");
}
END_TEST


START_TEST(Read_valid_string)
{
    static const struct
    {
        const char* data;
        const char* expected;
    }
    strings[] =
    {
        { "\"\"", "" },
        { "\" \"", " " },
        { "\"  \"", "  " },
        { "\"\\\"\"", "\"" },
        { "\"\\\\\"", "\\" },
        { "\"\\/\"", "/" },
        { "\"/\"", "/" },
        { "\"\\b\"", "\b" },
        { "\"\\f\"", "\f" },
        { "\"\\n\"", "\n" },
        { "\"\\r\"", "\r" },
        { "\"\\t\"", "\t" },
        { "\"abc def\"", "abc def" },
    };

    for (size_t i = 0; i < arr_size(strings); ++i)
    {
        char data[128] = "";
        sprintf(data, "%s x", strings[i].data);

        Streader* sr = init_with_cstr(data);
        char result[128] = "zzz";

        fail_if(!Streader_read_string(sr, 128, result),
                "Could not read string `%s`: %s",
                data, Streader_get_error_desc(sr));
        fail_if(strcmp(result, strings[i].expected) != 0,
                "Streader stored `%s` instead of `%s`",
                result, strings[i].expected);
        fail_if(!Streader_match_char(sr, 'x'),
                "Streader did not consume string `%s` correctly");
    }
}
END_TEST


Suite* Streader_suite(void)
{
    Suite* s = suite_create("Streader");

    const int timeout = 4;

#define BUILD_TCASE(name)                   \
    TCase* tc_##name = tcase_create(#name); \
    suite_add_tcase(s, tc_##name);          \
    tcase_set_timeout(tc_##name, timeout)

    BUILD_TCASE(init);
    BUILD_TCASE(match);

    BUILD_TCASE(read_null);
    BUILD_TCASE(read_bool);
    BUILD_TCASE(read_int);
    BUILD_TCASE(read_float);
    BUILD_TCASE(read_string);
    //BUILD_TCASE(read_tstamp);
    //BUILD_TCASE(read_piref);
    //BUILD_TCASE(read_list);
    //BUILD_TCASE(read_dict);
    //BUILD_TCASE(read_format);

#undef BUILD_TCASE

    tcase_add_test(tc_init, Initial_streader_has_no_error_set);

    tcase_add_test(tc_match, Matching_visible_characters_succeed);
    tcase_add_test(tc_match, Matching_wrong_characters_fails);
    tcase_add_test(tc_match, Characters_past_specified_length_are_ignored);
    tcase_add_test(tc_match, Matching_strings_requires_quotes_in_data);
    tcase_add_test(tc_match, Matching_strings_succeeds);
    tcase_add_test(tc_match, Matching_wrong_strings_fails);

    tcase_add_test(tc_read_null, Reading_null_consumes_data);
    tcase_add_test(tc_read_null, Null_token_with_trailing_garbage_is_rejected);

    tcase_add_test(tc_read_bool, Reading_bool_stores_correct_value);
    tcase_add_test(tc_read_bool, Bool_with_trailing_garbage_is_rejected);

    tcase_add_test(tc_read_int, Read_zero_int);
    tcase_add_test(tc_read_int, Read_nonzero_int);
    tcase_add_test(tc_read_int, Reading_too_large_int_in_magnitude_fails);

    tcase_add_test(tc_read_float, Read_zero_float);
    tcase_add_test(tc_read_float, Read_nonzero_float);
    tcase_add_test(tc_read_float, Whitespace_terminates_decimal_number);

    tcase_add_test(tc_read_string, Read_valid_string);

    return s;
}


int main(void)
{
    Suite* suite = Streader_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


