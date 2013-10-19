

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


#include <string.h>

#include <test_common.h>

#include <Streader.h>


#define init_with_cstr(s) Streader_init(STREADER_AUTO, (s), strlen((s)))


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
    for (size_t i = 0; i < sizeof(ones) / sizeof(*ones); ++i)
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
    for (size_t i = 0; i < sizeof(exprs) / sizeof(*exprs); ++i)
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

    for (size_t i = 0; i < sizeof(nums) / sizeof(*nums); ++i)
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
    fail_if(!Streader_read_bool(sr, &result),
            "Could not read a false value");
    fail_if(result != false,
            "Reading false stored %d", (int)result);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume the false value");

    sr = init_with_cstr("true x");
    result = false;
    fail_if(!Streader_read_bool(sr, &result),
            "Could not read a true value");
    fail_if(result != true,
            "Reading true stored %d", (int)result);
    fail_if(!Streader_match_char(sr, 'x'),
            "Streader did not consume the true value");
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
    //BUILD_TCASE(read_int);
    //BUILD_TCASE(read_float);
    //BUILD_TCASE(read_string);
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


