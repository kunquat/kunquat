

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

#include <Order.h>


Suite* Order_suite(void);


START_TEST (new)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < ORDERS_MAX; ++k)
        {
            int16_t ret = Order_get(order, i, k);
            fail_unless(ret == ORDER_NONE,
                    "Newly created Order contained %hd at subsong %hu, index %hu.",
                    (short int)ret,
                    (unsigned short int)i,
                    (unsigned short int)k);
        }
    }
    del_Order(order);
}
END_TEST


START_TEST (set_get)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, 0, 0, 0))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, 0, 7, 7))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, 0, 8, 8))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, 0, 33, 33))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, 0, ORDERS_MAX - 1, ORDERS_MAX - 1))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    if (!Order_set(order, SUBSONGS_MAX - 1, ORDERS_MAX - 1, SUBSONGS_MAX + ORDERS_MAX - 2))
    {
        fprintf(stderr, "Order_set() returned false -- out of memory?\n");
        abort();
    }
    for (int i = 0; i < SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < ORDERS_MAX; ++k)
        {
            int16_t ret = Order_get(order, i, k);
            if ((i == 0 && k == 0)
                    || (i == 0 && k == 7)
                    || (i == 0 && k == 8)
                    || (i == 0 && k == 33)
                    || (i == 0 && k == ORDERS_MAX - 1)
                    || (i == SUBSONGS_MAX - 1 && k == ORDERS_MAX - 1))
            {
                fail_unless(ret == i + k,
                        "Order contained %hd instead of %d at subsong %d, index %d.",
                        (short int)ret, i + k, i, k);
            }
            else
            {
                fail_unless(ret == ORDER_NONE,
                        "Order contained %hd instead of %d at subsong %d, index %d.",
                        (short int)ret, ORDER_NONE, i, k);
            }
        }
    }
    for (int i = 0; i < SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < ORDERS_MAX; ++k)
        {
            if (!Order_set(order, i, k, i + k))
            {
                fprintf(stderr, "Order_set() returned false -- out of memory?\n");
                abort();
            }
        }
    }
    for (int i = 0; i < SUBSONGS_MAX; ++i)
    {
        for (int k = 0; k < ORDERS_MAX; ++k)
        {
            int16_t ret = Order_get(order, i, k);
            fail_unless(ret == i + k,
                    "Order contained %hd instead of %d at subsong %d, index %d.",
                    (short int)ret, i + k, i, k);
        }
    }
    del_Order(order);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_order_null)
{
    Order_set(NULL, 0, 0, 0);
}
END_TEST

START_TEST (set_break_subsong_inv)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        return;
    }
    Order_set(order, SUBSONGS_MAX, 0, 0);
    del_Order(order);
}
END_TEST

START_TEST (set_break_index_inv)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        return;
    }
    Order_set(order, 0, ORDERS_MAX, 0);
    del_Order(order);
}
END_TEST

START_TEST (set_break_pat_inv)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        return;
    }
    Order_set(order, 0, 0, ORDER_NONE - 1);
    del_Order(order);
}
END_TEST

START_TEST (get_break_order_null)
{
    Order_get(NULL, 0, 0);
}
END_TEST

START_TEST (get_break_subsong_inv)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        return;
    }
    Order_get(order, SUBSONGS_MAX, 0);
    del_Order(order);
}
END_TEST

START_TEST (get_break_index_inv)
{
    Order* order = new_Order();
    if (order == NULL)
    {
        fprintf(stderr, "new_Order() returned NULL -- out of memory?\n");
        return;
    }
    Order_get(order, 0, ORDERS_MAX);
    del_Order(order);
}
END_TEST
#endif


Suite* Order_suite(void)
{
    Suite* s = suite_create("Order");
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
    tcase_add_test_raise_signal(tc_set_get, set_break_order_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_subsong_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_index_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, set_break_pat_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_set_get, get_break_order_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_subsong_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_set_get, get_break_index_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Order_suite();
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


