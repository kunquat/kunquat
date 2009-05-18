

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

#include <frame_t.h>
#include <Instrument.h>
#include <Ins_table.h>


Suite* Ins_table_suite(void);


START_TEST (new)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    fail_unless(Ins_table_get(table, 1) == NULL,
            "new_Ins_table() didn't create an empty table.");
    del_Ins_table(table);
    table = new_Ins_table(256);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    for (int i = 1; i <= 256; ++i)
    {
        fail_unless(Ins_table_get(table, i) == NULL,
                "new_Ins_table() didn't create an empty table.");
    }
    del_Ins_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_size_inv)
{
    new_Ins_table(0);
}
END_TEST
#endif


START_TEST (set_get)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins1 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins1 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins2 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins2 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins3 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins3 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins4 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins4 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Ins_table* table = new_Ins_table(256);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 8, ins1))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 8)
        {
            fail_unless(ret == ins1,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins1, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    if (!Ins_table_set(table, 9, ins2))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 8)
        {
            fail_unless(ret == ins1,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins1, i);
        }
        else if (i == 9)
        {
            fail_unless(ret == ins2,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins2, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    if (!Ins_table_set(table, 256, ins3))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 8)
        {
            fail_unless(ret == ins1,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins1, i);
        }
        else if (i == 9)
        {
            fail_unless(ret == ins2,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins2, i);
        }
        else if (i == 256)
        {
            fail_unless(ret == ins3,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins3, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    if (!Ins_table_set(table, 9, ins4))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 8)
        {
            fail_unless(ret == ins1,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins1, i);
        }
        else if (i == 9)
        {
            fail_unless(ret == ins4,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins4, i);
        }
        else if (i == 256)
        {
            fail_unless(ret == ins3,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins3, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    del_Ins_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_table_null)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_set(NULL, 1, ins);
    del_Instrument(ins);
}
END_TEST

START_TEST (set_break_index_inv1)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_set(table, 0, ins);
    del_Ins_table(table);
    del_Instrument(ins);
}
END_TEST

START_TEST (set_break_index_inv2)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_set(table, 2, ins);
    del_Ins_table(table);
    del_Instrument(ins);
}
END_TEST

START_TEST (set_break_ins_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_set(table, 1, NULL);
    del_Ins_table(table);
}
END_TEST

START_TEST (set_break_ins_dup)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_set(table, 1, ins);
    Ins_table_set(table, 1, ins);
    del_Ins_table(table);
    del_Instrument(ins);
}
END_TEST

START_TEST (get_break_table_null)
{
    Ins_table_get(NULL, 1);
}
END_TEST

START_TEST (get_break_index_inv1)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_get(table, 0);
    del_Ins_table(table);
}
END_TEST

START_TEST (get_break_index_inv2)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_get(table, 2);
    del_Ins_table(table);
}
END_TEST
#endif


START_TEST (ins_table_remove)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins1 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins1 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins2 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins2 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins3 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins3 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Ins_table* table = new_Ins_table(256);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    Ins_table_remove(table, 192);
    Ins_table_remove(table, 1);
    if (!Ins_table_set(table, 1, ins1))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 256, ins2))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 129, ins3))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    Ins_table_remove(table, 2);
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 1)
        {
            fail_unless(ret == ins1,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins1, i);
        }
        else if (i == 256)
        {
            fail_unless(ret == ins2,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins2, i);
        }
        else if (i == 129)
        {
            fail_unless(ret == ins3,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins3, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    Ins_table_remove(table, 1);
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 256)
        {
            fail_unless(ret == ins2,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins2, i);
        }
        else if (i == 129)
        {
            fail_unless(ret == ins3,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins3, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    Ins_table_remove(table, 256);
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        if (i == 129)
        {
            fail_unless(ret == ins3,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, ins3, i);
        }
        else
        {
            fail_unless(ret == NULL,
                    "Ins_table_get() returned %p instead of %p at index %d.",
                            ret, NULL, i);
        }
    }
    Ins_table_remove(table, 129);
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        fail_unless(ret == NULL,
                "Ins_table_get() returned %p instead of %p at index %d.",
                        ret, NULL, i);
    }
    del_Ins_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (remove_break_table_null)
{
    Ins_table_remove(NULL, 1);
}
END_TEST

START_TEST (remove_break_index_inv1)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_remove(table, 0);
    del_Ins_table(table);
}
END_TEST

START_TEST (remove_break_index_inv2)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Ins_table_remove(table, 2);
    del_Ins_table(table);
}
END_TEST
#endif


START_TEST (clear)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins1 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins1 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins2 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins2 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument* ins3 = new_Instrument(bufs, bufs, 2, 1, 1);
    if (ins3 == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Ins_table* table = new_Ins_table(256);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    Ins_table_clear(table);
    if (!Ins_table_set(table, 1, ins1))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 256, ins2))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 129, ins3))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    Ins_table_clear(table);
    for (int i = 1; i <= 256; ++i)
    {
        Instrument* ret = Ins_table_get(table, i);
        fail_unless(ret == NULL,
                "Ins_table_get() returned %p instead of %p at index %d.",
                        ret, NULL, i);
    }
    del_Ins_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (clear_break_table_null)
{
    Ins_table_clear(NULL);
}
END_TEST
#endif


Suite* Ins_table_suite(void)
{
    Suite* s = suite_create("Ins_table");
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
    tcase_add_test(tc_remove, ins_table_remove);
    tcase_add_test(tc_clear, clear);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break_size_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_set_get, set_break_table_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv1, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv2, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_ins_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_ins_dup, SIGABRT);

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
    Suite* s = Ins_table_suite();
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


