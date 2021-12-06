

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2021
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <test_common.h>

#include <mathnum/Tstamp.h>
#include <Pat_inst_ref.h>
#include <string/Streader.h>

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


#define init_with_cstr(s) Streader_init(STREADER_AUTO, (s), (int64_t)strlen((s)))

#define arr_size(arr) (sizeof(arr) / sizeof(*(arr)))


START_TEST(Initial_streader_has_no_error_set)
{
    const Streader* sr = init_with_cstr("");
    ck_assert_msg(!Streader_is_error_set(sr),
            "Streader initialised with an empty string has an error set");

    sr = init_with_cstr("]]]]");
    ck_assert_msg(!Streader_is_error_set(sr),
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

        ck_assert_msg(Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", ones[i]);
        ck_assert_msg(!Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", ones[i]);
        ck_assert_msg(!Streader_match_char(sr, '1'),
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

        ck_assert_msg(Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", exprs[i]);
        ck_assert_msg(!Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
        ck_assert_msg(Streader_match_char(sr, '+'),
                "Could not match '+' in \"%s\"", exprs[i]);
        ck_assert_msg(!Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
        ck_assert_msg(Streader_match_char(sr, '2'),
                "Could not match '2' in \"%s\"", exprs[i]);
        ck_assert_msg(!Streader_is_error_set(sr),
                "Error set after a successful match in \"%s\"", exprs[i]);
    }
}
END_TEST


START_TEST(Matching_wrong_characters_fails)
{
    Streader* sr = init_with_cstr("1");

    ck_assert_msg(!Streader_match_char(sr, '2'),
            "Matched '2' successfully in \"1\"");
    ck_assert_msg(Streader_is_error_set(sr),
            "No error set after a failed match");
    ck_assert_msg(!Streader_match_char(sr, '1'),
            "Match succeeded after an error");

    Streader_clear_error(sr);
    ck_assert_msg(!Streader_is_error_set(sr),
            "Streader_clear_error did not remove Streader error");
    ck_assert_msg(Streader_match_char(sr, '1'),
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

        ck_assert_msg(Streader_match_char(sr, '1'),
                "Could not match '1' in \"%s\"", nums[i].str);
        ck_assert_msg(Streader_match_char(sr, '2'),
                "Could not match '2' in \"%s\"", nums[i].str);
        ck_assert_msg(Streader_match_char(sr, '3'),
                "Could not match '3' in \"%s\"", nums[i].str);

        ck_assert_msg(!Streader_match_char(sr, '4'),
                "Matched '4' located in \"%s\" after given length %d",
                nums[i].str,
                nums[i].len);
    }
}
END_TEST


START_TEST(Matching_strings_requires_quotes_in_data)
{
    Streader* sr = init_with_cstr("abc");
    ck_assert_msg(!Streader_match_string(sr, "abc"),
            "Matched a string without double quotes in data");

    sr = init_with_cstr("abc\"");
    ck_assert_msg(!Streader_match_string(sr, "abc"),
            "Matched a string without opening double quote in data");

    sr = init_with_cstr("\"abc");
    ck_assert_msg(!Streader_match_string(sr, "abc"),
            "Matched a string without closing double quote in data");
}
END_TEST


START_TEST(Matching_strings_succeeds)
{
    Streader* sr = init_with_cstr("\"\"");
    ck_assert_msg(Streader_match_string(sr, ""), "Could not match empty string");

    sr = init_with_cstr("\" \"");
    ck_assert_msg(Streader_match_string(sr, " "),
            "Could not match a string with whitespace");

    sr = init_with_cstr("\"abc\"");
    ck_assert_msg(Streader_match_string(sr, "abc"),
            "Could not match the string \"abc\"");

    sr = init_with_cstr("\"\" \"\"");
    ck_assert_msg(Streader_match_string(sr, ""),
            "Could not match the first of two empty strings");
    ck_assert_msg(Streader_match_string(sr, ""),
            "Could not match the second of two empty strings");
    ck_assert_msg(!Streader_match_string(sr, ""),
            "Matched an empty string when end of data should have been reached");
}
END_TEST


START_TEST(Matching_wrong_strings_fails)
{
    Streader* sr = init_with_cstr("\"\"");
    ck_assert_msg(!Streader_match_string(sr, " "),
            "Empty string and string with whitespace were matched");

    sr = init_with_cstr("\" \"");
    ck_assert_msg(!Streader_match_string(sr, ""),
            "Empty string and string with whitespace were matched");
}
END_TEST


START_TEST(Reading_null_consumes_data)
{
    Streader* sr = init_with_cstr("null x");
    ck_assert_msg(Streader_read_null(sr), "Could not read a null value");
    ck_assert_msg(Streader_match_char(sr, 'x'),
            "Streader did not consume the null value");
}
END_TEST


START_TEST(Null_token_with_trailing_garbage_is_rejected)
{
    Streader* sr = init_with_cstr("nullz");
    ck_assert_msg(!Streader_read_null(sr),
            "Reading null token did not check for trailing garbage");
}
END_TEST


START_TEST(Reading_bool_stores_correct_value)
{
    Streader* sr = init_with_cstr("false x");
    bool result = true;
    ck_assert_msg(Streader_read_bool(sr, &result), "Could not read a false value");
    ck_assert_msg(result == false,
            "Reading false stored %d", (int)result);
    ck_assert_msg(Streader_match_char(sr, 'x'),
            "Streader did not consume the false value");

    sr = init_with_cstr("true x");
    result = false;
    ck_assert_msg(Streader_read_bool(sr, &result), "Could not read a true value");
    ck_assert_msg(result == true,
            "Reading true stored %d", (int)result);
    ck_assert_msg(Streader_match_char(sr, 'x'),
            "Streader did not consume the true value");
}
END_TEST


START_TEST(Bool_with_trailing_garbage_is_rejected)
{
    Streader* sr = init_with_cstr("falsez");
    bool result = false;
    ck_assert_msg(!Streader_read_bool(sr, &result),
            "Streader accepted falsez as a Boolean false value");

    sr = init_with_cstr("truez");
    ck_assert_msg(!Streader_read_bool(sr, &result),
            "Streader accepted truez as a Boolean true value");
}
END_TEST


START_TEST(Read_zero_int)
{
    Streader* sr = init_with_cstr("0 x");
    int64_t num = -1;
    ck_assert_msg(Streader_read_int(sr, &num), "Could not read 0");
    ck_assert_msg(num == 0, "Streader stored %" PRId64 " instead of 0", num);
    ck_assert_msg(Streader_match_char(sr, 'x'), "Streader did not consume 0 correctly");

    sr = init_with_cstr("-0 x");
    num = 1;
    ck_assert_msg(Streader_read_int(sr, &num), "Could not read -0");
    ck_assert_msg(num == 0, "Streader stored %" PRId64 " instead of 0", num);
    ck_assert_msg(Streader_match_char(sr, 'x'), "Streader did not consume -0 correctly");
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
        ck_assert_msg(Streader_read_int(sr, &num), "Could not read %" PRId64, nums[i]);
        ck_assert_msg(num == nums[i],
                "Streader stored %" PRId64 " instead of %" PRId64,
                num, nums[i]);
        ck_assert_msg(Streader_match_char(sr, 'x'),
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
        (void)len;
    }

    // Overflow data[0] and data[1] by 1
    ++data[0][strlen(data[0]) - 1];
    ++data[1][strlen(data[1]) - 1];

    // Overflow data[2] and data[3] by 10
    ++data[2][strlen(data[2]) - 2];
    ++data[3][strlen(data[3]) - 2];

    // Test reading
    for (int i = 0; i < 4; ++i)
    {
        Streader* sr = init_with_cstr(data[i]);
        int64_t num = 0;
        ck_assert_msg(!Streader_read_int(sr, &num),
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
        ck_assert_msg(Streader_read_float(sr, &num),
                "Could not read 0 from \"%s\": %s",
                zeros[i], Streader_get_error_desc(sr));
        ck_assert_msg(num == 0,
                "Streader stored %f instead of 0 from \"%s\"",
                num, zeros[i]);
        ck_assert_msg(Streader_match_char(sr, 'x'),
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
        ck_assert_msg(Streader_read_float(sr, &num),
                "Could not read -0 from \"%s\": %s",
                neg_zeros[i], Streader_get_error_desc(sr));
        ck_assert_msg(num == 0,
                "Streader stored %f instead of -0 from \"%s\"",
                num, neg_zeros[i]);
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume -0 from \"%s\" correctly",
                neg_zeros[i]);
    }
}
END_TEST


START_TEST(Read_nonzero_float)
{
    const double nums[] = { 0.5, 1.0, 1.5, -0.5, -1.0, -1.5, 0.0625, 10.0, };
    const char* formats[] =
    {
        "%.4f x",
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
            ck_assert_msg(Streader_read_float(sr, &num),
                    "Could not read float from \"%s\": %s",
                    data, Streader_get_error_desc(sr));
            ck_assert_msg(num == nums[i],
                    "Streader stored %f instead of %.6f from \"%s\"",
                    num, nums[i], data);
            ck_assert_msg(Streader_match_char(sr, 'x'),
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
    ck_assert_msg(!Streader_read_float(sr, &num),
            "Streader accepted \"- 1\" as a float");

    sr = init_with_cstr("-1 .5");
    num = NAN;
    ck_assert_msg(Streader_read_float(sr, &num),
            "Could not read float from \"-1 .5\": %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(num == -1, "Streader read %f instead of -1 from \"-1 .5\"", num);

    sr = init_with_cstr("-1. 5");
    num = NAN;
    ck_assert_msg(!Streader_read_float(sr, &num),
            "Streader accepted \"-1.\" as a float");

    sr = init_with_cstr("-1 e5");
    num = NAN;
    ck_assert_msg(Streader_read_float(sr, &num),
            "Could not read float from \"-1 e5\": %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(num == -1, "Streader read %f instead of -1 from \"-1 e5\"", num);

    sr = init_with_cstr("-1e 5");
    num = NAN;
    ck_assert_msg(!Streader_read_float(sr, &num),
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
        /*
        { "\"\\b\"", "\b" },
        { "\"\\f\"", "\f" },
        { "\"\\n\"", "\n" },
        { "\"\\r\"", "\r" },
        { "\"\\t\"", "\t" },
        */
        { "\"abc def\"", "abc def" },
    };

    for (size_t i = 0; i < arr_size(strings); ++i)
    {
        char data[128] = "";
        sprintf(data, "%s x", strings[i].data);

        Streader* sr = init_with_cstr(data);
        char result[128] = "zzz";

        ck_assert_msg(Streader_read_string(sr, 128, result),
                "Could not read string `%s`: %s",
                data, Streader_get_error_desc(sr));
        ck_assert_msg(strcmp(result, strings[i].expected) == 0,
                "Streader stored `%s` instead of `%s`",
                result, strings[i].expected);
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume string `%s` correctly", data);
    }
}
END_TEST


START_TEST(Reading_invalid_string_fails)
{
    const char* data[] =
    {
        "abc\"",
        "\"abc",
        "abc",
        "\"\\z\"",
        "\"\n\"",
    };

    for (size_t i = 0; i < arr_size(data); ++i)
    {
        Streader* sr = init_with_cstr(data[i]);
        char str[128] = "";
        ck_assert_msg(!Streader_read_string(sr, 128, str),
                "Streader accepted `%s` as a valid string",
                data[i]);
    }
}
END_TEST


START_TEST(Read_valid_tstamp)
{
    static const struct
    {
        const char* data;
        const Tstamp expected;
    }
    tstamps[] =
    {
        { "[0, 0]", { 0, 0 } },
        { "[2,5]",  { 2, 5 } },
        { " [2,5]", { 2, 5 } },
        { "[ 2,5]", { 2, 5 } },
        { "[2 ,5]", { 2, 5 } },
        { "[2, 5]", { 2, 5 } },
        { "[2,5 ]", { 2, 5 } },
        { "[-1, 3]", { -1, 3 } },
        { "[0, 882161279]", { 0, KQT_TSTAMP_BEAT - 1 } },
    };

    for (size_t i = 0; i < arr_size(tstamps); ++i)
    {
        char data[128] = "";
        sprintf(data, "%s x", tstamps[i].data);

        Streader* sr = init_with_cstr(data);
        Tstamp* result = Tstamp_set(TSTAMP_AUTO, 99, 99);

        ck_assert_msg(Streader_read_tstamp(sr, result),
                "Could not read timestamp " PRIts " from `%s`: %s",
                PRIVALts(tstamps[i].expected),
                data,
                Streader_get_error_desc(sr));
        ck_assert_msg(Tstamp_cmp(result, &tstamps[i].expected) == 0,
                "Streader stored " PRIts " instead of " PRIts
                    " when reading `%s`",
                PRIVALts(*result),
                PRIVALts(tstamps[i].expected),
                data);
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume timestamp from `%s` correctly", data);
    }
}
END_TEST


START_TEST(Reading_invalid_tstamp_fails)
{
    const char* data[] =
    {
        "0, 0]",
        "[0 0]",
        "[0, 0",
        "[0, -1]",
        "[0, 882161280]",
    };

    for (size_t i = 0; i < arr_size(data); ++i)
    {
        Streader* sr = init_with_cstr(data[i]);
        Tstamp* result = TSTAMP_AUTO;

        ck_assert_msg(!Streader_read_tstamp(sr, result),
                "Streader accepted `%s` as a valid timestamp",
                data[i]);
    }
}
END_TEST


START_TEST(Read_valid_piref)
{
    static const struct
    {
        const char* data;
        const Pat_inst_ref expected;
    }
    pirefs[] =
    {
        { "[0, 0]", { 0, 0 } },
        { "[2,5]",  { 2, 5 } },
        { " [2,5]", { 2, 5 } },
        { "[ 2,5]", { 2, 5 } },
        { "[2 ,5]", { 2, 5 } },
        { "[2, 5]", { 2, 5 } },
        { "[2,5 ]", { 2, 5 } },
        { "[0, 1023]", { 0, KQT_PAT_INSTANCES_MAX - 1 } },
        { "[1023, 0]", { KQT_PATTERNS_MAX - 1, 0 } },
        { "[1023, 1023]", { KQT_PATTERNS_MAX - 1, KQT_PAT_INSTANCES_MAX - 1 } },
    };

    for (size_t i = 0; i < arr_size(pirefs); ++i)
    {
        char data[128] = "";
        sprintf(data, "%s x", pirefs[i].data);

        Streader* sr = init_with_cstr(data);
        Pat_inst_ref* result = PAT_INST_REF_AUTO;

        ck_assert_msg(Streader_read_piref(sr, result),
                "Could not read pattern instance refernce " PRIpi
                    " from `%s`: %s",
                PRIVALpi(pirefs[i].expected),
                data,
                Streader_get_error_desc(sr));
        ck_assert_msg(Pat_inst_ref_cmp(result, &pirefs[i].expected) == 0,
                "Streader stored " PRIpi " instead of " PRIpi
                    " when reading `%s`",
                PRIVALpi(*result),
                PRIVALpi(pirefs[i].expected),
                data);
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume pattern instance from `%s` correctly",
                data);
    }
}
END_TEST


#define make_str(x) #x

START_TEST(Reading_invalid_piref_fails)
{
    const char* data[] =
    {
        "0, 0]",
        "[0 0]",
        "[0, 0",
        "[0, -1]",
        "[-1, 0]",
        "[0, " make_str(KQT_PAT_INSTANCES_MAX) "]",
        "[" make_str(KQT_PATTERNS_MAX) ", 0]",
    };

    for (size_t i = 0; i < arr_size(data); ++i)
    {
        Streader* sr = init_with_cstr(data[i]);
        Pat_inst_ref* result = PAT_INST_REF_AUTO;

        ck_assert_msg(!Streader_read_piref(sr, result),
                "Streader accepted `%s` as a valid pattern instance reference",
                data[i]);
    }
}
END_TEST

#undef make_str


START_TEST(Read_empty_list)
{
    static const char* lists[] =
    {
        "[] x",
        "[ ]x",
        "[ ] x",
    };

    for (size_t i = 0; i < arr_size(lists); ++i)
    {
        Streader* sr = init_with_cstr(lists[i]);
        ck_assert_msg(Streader_read_list(sr, NULL, NULL),
                "Could not read empty list from `%s`: %s",
                lists[i],
                Streader_get_error_desc(sr));
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume empty list from `%s` correctly",
                lists[i]);
    }
}
END_TEST


#define item_count 4

static bool inc_doubled_int(Streader* sr, int32_t index, void* userdata)
{
    ck_assert_msg(sr != NULL, "Callback did not get a Streader");
    ck_assert_msg(!Streader_is_error_set(sr),
            "Callback was called with Streader error set: %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(index >= 0,
            "Callback got a negative item index (%" PRId32 ")",
            index);
    ck_assert_msg(index < item_count,
            "Callback got too large an index (%" PRId32 ")",
            index);
    ck_assert_msg(userdata != NULL, "Callback did not get userdata");

    int64_t num = 0;
    ck_assert_msg(Streader_read_int(sr, &num),
            "Could not read integer from list index %" PRId32 ": %s",
            index, Streader_get_error_desc(sr));
    ck_assert_msg(num == index * 2,
            "Unexpected list item %" PRId64 " (expected %" PRId32 ")",
            num, index * 2);

    int* nums = userdata;
    nums[index] = (int)num + 1;

    return true;
}

START_TEST(Read_list_of_numbers)
{
    static const char* lists[] =
    {
        "[] x",
        "[0] x",
        "[0, 2] x",
        "[0, 2, 4] x",
        "[0, 2, 4, 6] x",
    };

    for (size_t i = 0; i < arr_size(lists); ++i)
    {
        int nums[item_count + 1] = { 99 };

        Streader* sr = init_with_cstr(lists[i]);
        ck_assert_msg(Streader_read_list(sr, inc_doubled_int, nums),
                "Could not read list from `%s`: %s",
                lists[i], Streader_get_error_desc(sr));

        for (int k = 0; (size_t)k < i; ++k)
        {
            ck_assert_msg(nums[k] == k * 2 + 1,
                    "Reading of list stored %d instead of %d",
                    nums[k], k * 2 + 1);
        }

        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume list from `%s` correctly",
                lists[i]);
    }
}
END_TEST


static bool check_adjusted_tstamp(Streader* sr, int32_t index, void* userdata)
{
    ck_assert_msg(sr != NULL, "Callback did not get a Streader");
    ck_assert_msg(!Streader_is_error_set(sr),
            "Callback was called with Streader error set: %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(index >= 0,
            "Callback got a negative item index (%" PRId32 ")",
            index);
    ck_assert_msg(index < item_count,
            "Callback got too large an index (%" PRId32 ")",
            index);
    ck_assert_msg(userdata == NULL, "Callback got unexpected userdata");

    Tstamp* ts = TSTAMP_AUTO;
    ck_assert_msg(Streader_read_tstamp(sr, ts),
            "Could not read timestamp from list index %" PRId32 ": %s",
            index, Streader_get_error_desc(sr));
    ck_assert_msg((Tstamp_get_beats(ts) == index + 10) &&
                (Tstamp_get_rem(ts) == index + 100),
            "Unexpected list item " PRIts " (expected (%d, %d))",
            PRIVALts(*ts), (int)index + 10, (int)index + 100);

    return true;
}

START_TEST(Read_list_of_tstamps)
{
    static const char* lists[] =
    {
        "[]",
        "[[10, 100]]",
        "[[10, 100], [11, 101]]",
        "[[10, 100], [11, 101], [12, 102]]",
        "[[10, 100], [11, 101], [12, 102], [13, 103]]",
    };

    for (size_t i = 0; i < arr_size(lists); ++i)
    {
        Streader* sr = init_with_cstr(lists[i]);
        ck_assert_msg(Streader_read_list(sr, check_adjusted_tstamp, NULL),
                "Could not read list from `%s`: %s",
                lists[i], Streader_get_error_desc(sr));
    }
}
END_TEST

#undef max_index


START_TEST(Callback_must_be_specified_for_nonempty_lists)
{
    Streader* sr = init_with_cstr("[[]]");
    int dummy = 0;
    ck_assert_msg(!Streader_read_list(sr, NULL, &dummy),
            "Reading of non-empty list succeeded without callback");
}
END_TEST


static bool fail_at_index_2(Streader* sr, int32_t index, void* userdata)
{
    (void)sr;
    (void)userdata;

    ck_assert_msg(index <= 2, "List processing continued after failure");

    if (index == 2)
        return false;

    ck_assert_msg(Streader_read_int(sr, NULL),
            "Could not read an integer from list: %s",
            Streader_get_error_desc(sr));

    return true;
}

START_TEST(Callback_failure_interrupts_list_reading)
{
    static const char* lists[] =
    {
        "[]",
        "[0]",
        "[0, 0]",
        "[0, 0, 0]",
        "[0, 0, 0, 0]",
    };

    for (size_t i = 0; i < arr_size(lists); ++i)
    {
        Streader* sr = init_with_cstr(lists[i]);
        if (i <= 2)
        {
            ck_assert_msg(Streader_read_list(sr, fail_at_index_2, NULL),
                    "Could not read list from `%s`: %s",
                    lists[i], Streader_get_error_desc(sr));
        }
        else
        {
            ck_assert_msg(!Streader_read_list(sr, fail_at_index_2, NULL),
                    "List reading continued successfully after an error"
                        " (list length %zd)",
                    i);
        }
    }
}
END_TEST


START_TEST(Read_empty_dict)
{
    static const char* dicts[] =
    {
        "{} x",
        "{ }x",
        "{ } x",
    };

    for (size_t i = 0; i < arr_size(dicts); ++i)
    {
        Streader* sr = init_with_cstr(dicts[i]);
        ck_assert_msg(Streader_read_dict(sr, NULL, NULL),
                "Could not read empty dictionary from `%s`: %s",
                dicts[i],
                Streader_get_error_desc(sr));
        ck_assert_msg(Streader_match_char(sr, 'x'),
                "Streader did not consume empty dictionary from `%s` correctly",
                dicts[i]);
    }
}
END_TEST


static bool fill_array_index(Streader* sr, const char* key, void* userdata)
{
    ck_assert_msg(sr != NULL, "Callback did not get a Streader");
    ck_assert_msg(!Streader_is_error_set(sr),
            "Callback was called with Streader error set: %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(key != NULL,
            "Callback did not get a key");
    ck_assert_msg(strlen(key) == 1 && strchr("0123", key[0]) != NULL,
            "Callback got an unexpected key `%s`",
            key);
    ck_assert_msg(userdata != NULL, "Callback did not get userdata");

    int arr_index = key[0] - '0';
    assert(arr_index >= 0);
    assert(arr_index < 4);
    int* array = userdata;

    int64_t num = 0;
    ck_assert_msg(Streader_read_int(sr, &num),
            "Could not read integer from dictionary: %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(num == arr_index + 10,
            "Unexpected value %" PRId64 " (expected %d)",
            num, arr_index + 10);

    array[arr_index] = (int)num;

    return true;
}

START_TEST(Read_dict_of_numbers)
{
    static const char* dicts[] =
    {
        "{}",
        "{ \"0\": 10 }",
        "{ \"1\": 11, \"0\": 10 }",
        "{ \"0\": 10, \"2\": 12, \"1\": 11 }",
        "{ \"2\": 12, \"1\": 11, \"3\": 13, \"0\": 10 }",
    };

    for (size_t i = 0; i < arr_size(dicts); ++i)
    {
        Streader* sr = init_with_cstr(dicts[i]);
        int nums[4] = { 99 };

        ck_assert_msg(Streader_read_dict(sr, fill_array_index, nums),
                "Could not read dictionary from `%s`: %s",
                dicts[i], Streader_get_error_desc(sr));

        for (int k = 0; (size_t)k < i; ++k)
        {
            ck_assert_msg(nums[k] == k + 10,
                    "Unexpected value at index %d: %d (expected %d)",
                    k, nums[k], k + 10);
        }
    }
}
END_TEST


START_TEST(Callback_must_be_specified_for_nonempty_dicts)
{
    Streader* sr = init_with_cstr("{ \"key\": 0 }");
    int dummy = 0;
    ck_assert_msg(!Streader_read_dict(sr, NULL, &dummy),
            "Reading of non-empty dict succeeded without callback");
}
END_TEST


static bool fail_at_key_2(Streader* sr, const char* key, void* userdata)
{
    (void)sr;
    (void)userdata;

    ck_assert_msg(strcmp(key, "2") <= 0,
            "Dictionary processing continued after failure");

    if (strcmp(key, "2") == 0)
        return false;

    ck_assert_msg(Streader_read_float(sr, NULL),
            "Could not read a float from dictionary: %s",
            Streader_get_error_desc(sr));

    return true;
}

START_TEST(Callback_failure_interrupts_dict_reading)
{
    static const char* dicts[] =
    {
        "{}",
        "{ \"0\": 1 }",
        "{ \"1\": 1, \"0\": 0.5 }",
        "{ \"0\": 1.5, \"1\": 2, \"2\": 3 }",
        "{ \"1\": 10, \"2\": 20, \"3\": 30, \"4\": 40 }",
    };

    for (size_t i = 0; i < arr_size(dicts); ++i)
    {
        Streader* sr = init_with_cstr(dicts[i]);
        if (i <= 2)
        {
            ck_assert_msg(Streader_read_dict(sr, fail_at_key_2, NULL),
                    "Could not read dictionary from `%s`: %s",
                    dicts[i], Streader_get_error_desc(sr));
        }
        else
        {
            ck_assert_msg(!Streader_read_dict(sr, fail_at_key_2, NULL),
                    "Dictionary reading continued successfully after an error"
                        " (dictionary size %zd)",
                    i);
        }
    }
}
END_TEST


static bool readf_list(Streader* sr, int32_t index, void* userdata)
{
    (void)index;

    if (userdata == NULL)
        return false;

    int* dest = userdata;
    int64_t result = 0;

    if (!Streader_read_int(sr, &result))
        return false;

    *dest = (int)result;

    return true;
}

static bool readf_dict(Streader* sr, const char* key, void* userdata)
{
    if (userdata == NULL)
        return false;

    if (strcmp(key, "ten") != 0)
        return false;

    int* dest = userdata;
    int64_t result = 0;

    if (!Streader_read_int(sr, &result))
        return false;

    *dest = (int)result;

    return true;
}

START_TEST(Read_formatted_input)
{
    static const char* data =
        "[null, true] - \"abcde\": [4,6][7,8] -- [9] 2 { \"ten\": 10 } 3.5";

    Streader* sr = init_with_cstr(data);
    bool b = false;
    char s[10] = "";
    Tstamp* ts = TSTAMP_AUTO;
    Pat_inst_ref* piref = PAT_INST_REF_AUTO;
    int list_userdata = 0;
    int64_t i = 0;
    int dict_userdata = 0;
    double f = 0.0;

    Streader_readf(
            sr,
            "[%n,%b] -%s :%t%p--%l%i %d %f",
            &b, READF_STR(4, s), ts, piref,
            readf_list, &list_userdata,
            &i,
            readf_dict, &dict_userdata,
            &f);

    ck_assert_msg(!Streader_is_error_set(sr),
            "Could not read formatted input: %s",
            Streader_get_error_desc(sr));
    ck_assert_msg(b == true,
            "Formatted reader did not store a boolean value correctly");
    ck_assert_msg(strcmp(s, "abc") == 0,
            "Formatted reader stored `%s` instead of `%s`",
            s, "abc");
    ck_assert_msg(Tstamp_cmp(ts, Tstamp_set(TSTAMP_AUTO, 4, 6)) == 0,
            "Formatted reader stored " PRIts " instead of " PRIts,
            PRIVALts(*ts), PRIVALts(*Tstamp_set(TSTAMP_AUTO, 4, 6)));
    ck_assert_msg(Pat_inst_ref_cmp(piref, &(Pat_inst_ref){ 7, 8 }) == 0,
            "Formatted reader stored " PRIpi " instead of (7, 8)",
            PRIVALpi(*piref));
    ck_assert_msg(list_userdata == 9,
            "Formatted reader stored list value %d instead of 9",
            list_userdata);
    ck_assert_msg(i == 2,
            "Formatted reader stored integer %" PRId64 " instead of 2",
            i);
    ck_assert_msg(dict_userdata == 10,
            "Formatted reader stored dict value %d instead of 10",
            dict_userdata);
    ck_assert_msg(f == 3.5,
            "Formatted reader stored floating-point value %f instead of 3.5",
            f);
}
END_TEST


static Suite* Streader_suite(void)
{
    Suite* s = suite_create("Streader");

    const int timeout = DEFAULT_TIMEOUT;

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
    BUILD_TCASE(read_tstamp);
    BUILD_TCASE(read_piref);
    BUILD_TCASE(read_list);
    BUILD_TCASE(read_dict);
    BUILD_TCASE(read_format);

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
    tcase_add_test(tc_read_string, Reading_invalid_string_fails);

    tcase_add_test(tc_read_tstamp, Read_valid_tstamp);
    tcase_add_test(tc_read_tstamp, Reading_invalid_tstamp_fails);

    tcase_add_test(tc_read_piref, Read_valid_piref);
    tcase_add_test(tc_read_piref, Reading_invalid_piref_fails);

    tcase_add_test(tc_read_list, Read_empty_list);
    tcase_add_test(tc_read_list, Read_list_of_numbers);
    tcase_add_test(tc_read_list, Read_list_of_tstamps);
    tcase_add_test(tc_read_list, Callback_must_be_specified_for_nonempty_lists);
    tcase_add_test(tc_read_list, Callback_failure_interrupts_list_reading);

    tcase_add_test(tc_read_dict, Read_empty_dict);
    tcase_add_test(tc_read_dict, Read_dict_of_numbers);
    tcase_add_test(tc_read_dict, Callback_must_be_specified_for_nonempty_dicts);
    tcase_add_test(tc_read_dict, Callback_failure_interrupts_dict_reading);

    tcase_add_test(tc_read_format, Read_formatted_input);

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


