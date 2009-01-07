

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>

#include <check.h>

#include <Pattern.h>
#include <Pat_table.h>


Suite* Pat_table_suite(void);


START_TEST (new)
{
    Pat_table* table = new_Pat_table(1024);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < 1024; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == NULL,
                "Newly created Pattern table contained %p at index %d.",
                ret, i);
    }
    del_Pat_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_size_inv)
{
    new_Pat_table(0);
}
END_TEST
#endif


START_TEST (set_get)
{
    Pattern* pats[128] = { NULL };
    Pat_table* table = new_Pat_table(128);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        abort();
    }
    pats[0] = new_Pattern();
    if (pats[0] == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    pats[7] = new_Pattern();
    if (pats[7] == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    pats[8] = new_Pattern();
    if (pats[8] == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    pats[33] = new_Pattern();
    if (pats[33] == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    pats[127] = new_Pattern();
    if (pats[127] == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Pat_table_set(table, 0, pats[0]))
    {
        fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Pat_table_set(table, 7, pats[7]))
    {
        fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Pat_table_set(table, 8, pats[8]))
    {
        fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Pat_table_set(table, 33, pats[33]))
    {
        fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Pat_table_set(table, 127, pats[127]))
    {
        fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < 128; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == pats[i],
                "Pattern table contained %p instead of %p at index %d.",
                ret, pats[i], i);
    }
    for (int i = 0; i < 128; ++i)
    {
        if (pats[i] == NULL)
        {
            pats[i] = new_Pattern();
            if (pats[i] == NULL)
            {
                fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
                abort();
            }
            if (!Pat_table_set(table, i, pats[i]))
            {
                fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
                abort();
            }
        }
    }
    for (int i = 0; i < 128; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == pats[i],
                "Pattern table contained %p instead of %p at index %d.",
                ret, pats[i], i);
    }
    del_Pat_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_table_null)
{
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_set(NULL, 0, pat);
    del_Pattern(pat);
}
END_TEST

START_TEST (set_break_index_inv1)
{
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_set(table, -1, pat);
    del_Pat_table(table);
}
END_TEST

START_TEST (set_break_index_inv2)
{
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_set(table, 16, pat);
    del_Pat_table(table);
}
END_TEST

START_TEST (set_break_pat_null)
{
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_set(table, 16, NULL);
    del_Pat_table(table);
}
END_TEST

START_TEST (get_break_table_null)
{
    Pat_table_get(NULL, 0);
}
END_TEST

START_TEST (get_break_index_inv1)
{
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_get(table, -1);
    del_Pat_table(table);
}
END_TEST

START_TEST (get_break_index_inv2)
{
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_get(table, 16);
    del_Pat_table(table);
}
END_TEST
#endif


START_TEST (remove_pat)
{
    Pattern* pats[16] = { NULL };
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < 16; ++i)
    {
        Pat_table_remove(table, i);
    }
    for (int i = 0; i < 16; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == NULL,
                "Pattern table contained %p instead of %p at index %d.",
                ret, NULL, i);
    }
    for (int i = 0; i < 16; ++i)
    {
        pats[i] = new_Pattern();
        if (pats[i] == NULL)
        {
            fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Pat_table_set(table, i, pats[i]))
        {
            fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
            abort();
        }
    }
    for (int i = 0; i < 16; ++i)
    {
        Pat_table_remove(table, i);
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == NULL,
                "Pattern table contained %p instead of %p at index %d.",
                ret, NULL, i);
    }
    del_Pat_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (remove_break_table_null)
{
    Pat_table_remove(NULL, 0);
}
END_TEST

START_TEST (remove_break_index_inv1)
{
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_remove(table, 16);
    del_Pat_table(table);
}
END_TEST

START_TEST (remove_break_index_inv2)
{
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        return;
    }
    Pat_table_remove(table, -1);
    del_Pat_table(table);
}
END_TEST
#endif


START_TEST (clear)
{
    Pattern* pats[16] = { NULL };
    Pat_table* table = new_Pat_table(16);
    if (table == NULL)
    {
        fprintf(stderr, "new_Pat_table() returned NULL -- out of memory?\n");
        abort();
    }
    Pat_table_clear(table);
    for (int i = 0; i < 16; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == NULL,
                "Pattern table contained %p instead of %p at index %d.",
                ret, NULL, i);
    }
    for (int i = 0; i < 16; ++i)
    {
        pats[i] = new_Pattern();
        if (pats[i] == NULL)
        {
            fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Pat_table_set(table, i, pats[i]))
        {
            fprintf(stderr, "Pat_table_set() returned false -- out of memory?\n");
            abort();
        }
    }
    Pat_table_clear(table);
    for (int i = 0; i < 16; ++i)
    {
        Pattern* ret = Pat_table_get(table, i);
        fail_unless(ret == NULL,
                "Pattern table contained %p instead of %p at index %d.",
                ret, NULL, i);
    }
    del_Pat_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (clear_break_table_null)
{
    Pat_table_clear(NULL);
}
END_TEST
#endif


Suite* Pat_table_suite(void)
{
    Suite* s = suite_create("Pat_table");
    TCase* tc_new = tcase_create("new");
    TCase* tc_set_get = tcase_create("set_get");
    TCase* tc_remove = tcase_create("remove");
    TCase* tc_clear = tcase_create("clear");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_set_get);
    suite_add_tcase(s, tc_remove);
    suite_add_tcase(s, tc_clear);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_set_get, timeout);
    tcase_set_timeout(tc_remove, timeout);
    tcase_set_timeout(tc_clear, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_set_get, set_get);
    tcase_add_test(tc_remove, remove_pat);
    tcase_add_test(tc_clear, clear);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break_size_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_set_get, set_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_pat_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_index_inv1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_index_inv2, SIGABRT);

    tcase_add_test_raise_signal(tc_remove, remove_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_remove, remove_break_index_inv1, SIGABRT);
    tcase_add_test_raise_signal(tc_remove, remove_break_index_inv2, SIGABRT);
    
    tcase_add_test_raise_signal(tc_clear, clear_break_table_null, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Pat_table_suite();
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


