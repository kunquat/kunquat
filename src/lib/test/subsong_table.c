

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>

#include <check.h>

#include <Subsong_table.h>


Suite* Subsong_table_suite(void);


START_TEST (new)
{
    Subsong_table* table = new_Subsong_table();
    if (table == NULL)
    {
        fprintf(stderr, "new_Subsong_table() returned NULL -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < KQT_SECTIONS_MAX; ++k)
        {
            int16_t ret = KQT_SECTION_NONE;
            Subsong* ss = Subsong_table_get(table, i);
            if (ss != NULL)
            {
                ret = Subsong_get(ss, k);
            }
            fail_unless(ret == KQT_SECTION_NONE,
                    "Newly created Subsong table contained %hd at subsong %hu, index %hu.",
                    (short int)ret,
                    (unsigned short int)i,
                    (unsigned short int)k);
        }
    }
    del_Subsong_table(table);
}
END_TEST


START_TEST (set_get)
{
    Subsong_table* table = new_Subsong_table();
    if (table == NULL)
    {
        fprintf(stderr, "new_Subsong_table() returned NULL -- out of memory?\n");
        abort();
    }
    Subsong* ss = new_Subsong();
    if (ss == NULL)
    {
        fprintf(stderr, "new_Subsong() returned NULL -- out of memory?\n");
        abort();
    }
    Subsong* ss_end = new_Subsong();
    if (ss_end == NULL)
    {
        fprintf(stderr, "new_Subsong() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Subsong_table_set(table, 0, ss))
    {
        fprintf(stderr, "Subsong_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_table_set(table, KQT_SUBSONGS_MAX - 1, ss_end))
    {
        fprintf(stderr, "Subsong_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss, 0, 0))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss, 7, 7))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss, 8, 8))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss, 33, 33))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss, KQT_SECTIONS_MAX - 1, KQT_SECTIONS_MAX - 1))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Subsong_set(ss_end, KQT_SECTIONS_MAX - 1, 1 + KQT_SECTIONS_MAX - 1))
    {
        fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < KQT_SECTIONS_MAX; ++k)
        {
            int16_t ret = KQT_SECTION_NONE;
            Subsong* subsong = Subsong_table_get(table, i);
            if (subsong != NULL)
            {
                ret = Subsong_get(subsong, k);
            }
            if ((i == 0 && k == 0)
                    || (i == 0 && k == 7)
                    || (i == 0 && k == 8)
                    || (i == 0 && k == 33)
                    || (i == 0 && k == KQT_SECTIONS_MAX - 1)
//                    || (i == 1 && k == KQT_SECTIONS_MAX - 1)
                    )
            {
                fail_unless(ret == i + k,
                        "Subsong table contained %hd instead of %d at subsong %d, index %d.",
                        (short int)ret, i + k, i, k);
            }
            else
            {
                fail_unless(ret == KQT_SECTION_NONE,
                        "Subsong table contained %hd instead of %d at subsong %d, index %d.",
                        (short int)ret, KQT_SECTION_NONE, i, k);
            }
        }
    }
    for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
    {
        Subsong* subsong = new_Subsong();
        if (subsong == NULL)
        {
            fprintf(stderr, "new_Subsong() returned false -- out of memory?\n");
            abort();
        }
        if (Subsong_table_set(table, i, subsong) < 0)
        {
            fprintf(stderr, "Subsong_table_set() returned negative -- out of memory?\n");
            abort();
        }
        for (int k = 0; k < KQT_SECTIONS_MAX; ++k)
        {
            if (!Subsong_set(subsong, k, i + k))
            {
                fprintf(stderr, "Subsong_set() returned false -- out of memory?\n");
                abort();
            }
        }
    }
    for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < KQT_SECTIONS_MAX; ++k)
        {
            int16_t ret = KQT_SECTION_NONE;
            Subsong* subsong = Subsong_table_get(table, i);
            if (subsong != NULL)
            {
                ret = Subsong_get(subsong, k);
            }
            fail_unless(ret == i + k,
                    "Subsong table contained %hd instead of %d at subsong %d, index %d.",
                    (short int)ret, i + k, i, k);
        }
    }
    del_Subsong_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_table_null)
{
    Subsong* ss = new_Subsong();
    if (ss == NULL)
    {
        fprintf(stderr, "new_Subsong() returned NULL -- out of memory?\n");
        abort();
    }
    Subsong_table_set(NULL, 0, ss);
    del_Subsong(ss);
}
END_TEST

START_TEST (set_break_index_inv)
{
    Subsong* ss = new_Subsong();
    if (ss == NULL)
    {
        fprintf(stderr, "new_Subsong() returned NULL -- out of memory?\n");
        abort();
    }
    Subsong_table* table = new_Subsong_table();
    if (table == NULL)
    {
        fprintf(stderr, "new_Subsong_table() returned NULL -- out of memory?\n");
        return;
    }
    Subsong_table_set(table, KQT_SUBSONGS_MAX, ss);
    del_Subsong_table(table);
    del_Subsong(ss);
}
END_TEST

START_TEST (set_break_subsong_inv)
{
    Subsong_table* table = new_Subsong_table();
    if (table == NULL)
    {
        fprintf(stderr, "new_Subsong_table() returned NULL -- out of memory?\n");
        return;
    }
    Subsong_table_set(table, 0, NULL);
    del_Subsong_table(table);
}
END_TEST

START_TEST (get_break_table_null)
{
    Subsong_table_get(NULL, 0);
}
END_TEST

START_TEST (get_break_index_inv)
{
    Subsong_table* table = new_Subsong_table();
    if (table == NULL)
    {
        fprintf(stderr, "new_Subsong_table() returned NULL -- out of memory?\n");
        return;
    }
    Subsong_table_get(table, KQT_SUBSONGS_MAX);
    del_Subsong_table(table);
}
END_TEST
#endif


Suite* Subsong_table_suite(void)
{
    Suite* s = suite_create("Subsong_table");
    TCase* tc_new = tcase_create("new");
    TCase* tc_set_get = tcase_create("set_get");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_set_get);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_set_get, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_set_get, set_get);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_set_get, set_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_subsong_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_set_get, get_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_index_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Subsong_table_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    if (fail_count > 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


