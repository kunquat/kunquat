

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

#include <Event.h>
#include <Reltime.h>
#include <Column.h>


Suite* Column_suite(void);


START_TEST (new)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    del_Column(col);
    col = new_Column(Reltime_init(RELTIME_AUTO));
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    fail_unless(Column_iter_get_next(citer) == NULL,
            "new_Column() created a non-empty Column.");
    fail_unless(Column_iter_get(citer, Reltime_init(RELTIME_AUTO)) == NULL,
            "new_Column() created a non-empty Column.");
    del_Column_iter(citer);
    del_Column(col);
}
END_TEST


START_TEST (ins)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    for (int64_t i = 4; i >= 0; --i)
    {
        Event* event = new_Event(Reltime_set(RELTIME_AUTO, i, 0), EVENT_TYPE_NOTE_ON);
        if (event == NULL)
        {
            fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Column_ins(col, event))
        {
            fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
            abort();
        }
        Event* ret = Column_iter_get(citer, Reltime_set(RELTIME_AUTO, i, 0));
        fail_if(ret == NULL,
                "Couldn't find the Event inserted at beat %lld.", (long long)i);
        int64_t beat = Event_pos(ret)->beats;
        fail_unless(beat == i,
                "Column_get() returned Event %lld instead of %lld.",
                (long long)beat, (long long)i);
        for (int64_t k = i + 1; k <= 4; ++k)
        {
            ret = Column_iter_get_next(citer);
            fail_if(ret == NULL,
                    "Couldn't find the Event inserted at beat %lld.", (long long)k);
            beat = Event_pos(ret)->beats;
            fail_unless(beat == k,
                    "Column_get() returned Event %lld instead of %lld.",
                    (long long)beat, (long long)k);
        }
    }
    Event* event = new_Event(Reltime_set(RELTIME_AUTO, 2, 0), EVENT_TYPE_NOTE_ON);
    if (event == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Column_ins(col, event))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    Event* ret = Column_iter_get(citer, Reltime_set(RELTIME_AUTO, 2, 0));
    fail_if(ret == NULL,
            "Couldn't find the Event inserted at beat 2.");
    fail_if(ret == event,
            "Column_get() returned the wrong Event at beat 2.");
    int64_t beat = Event_pos(ret)->beats;
    fail_unless(beat == 2,
            "Column_get() returned Event %lld instead of 2.", (long long)beat);
    ret = Column_iter_get_next(citer);
    fail_if(ret == NULL,
            "Couldn't find the Event inserted at beat 2.");
    fail_unless(ret == event,
            "Column_get() returned the wrong Event at beat 2.");
    beat = Event_pos(ret)->beats;
    fail_unless(beat == 2,
            "Column_get() returned Event %lld instead of 2.", (long long)beat);
    del_Column_iter(citer);
    del_Column(col);
}
END_TEST

#ifndef NDEBUG
START_TEST (ins_break_col_null)
{
    Event* event = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (event == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        return;
    }
    Column_ins(NULL, event);
    del_Event(event);
}
END_TEST

START_TEST (ins_break_event_null)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_ins(col, NULL);
    del_Column(col);
}
END_TEST
#endif


START_TEST (col_remove)
{
    Event* events[7] = { NULL };
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    events[0] = new_Event(Reltime_set(RELTIME_AUTO, 0, 0), EVENT_TYPE_NOTE_ON);
    if (events[0] == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Column_ins(col, events[0]))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    fail_unless(Column_remove(col, events[0]),
            "Failed to remove a single Event.");
    fail_unless(Column_iter_get(citer, Reltime_set(RELTIME_AUTO, 0, 0)) == NULL,
            "Column_get() returned an Event from an empty Column.");
    for (int64_t i = 0; i < 7; ++i)
    {
        events[i] = new_Event(Reltime_set(RELTIME_AUTO, i, 0), EVENT_TYPE_NOTE_ON);
        if (events[i] == NULL)
        {
            fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Column_ins(col, events[i]))
        {
            fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
            abort();
        }
        Event* ret = Column_iter_get(citer, Reltime_set(RELTIME_AUTO, i, 0));
        fail_if(ret == NULL,
                "Couldn't find the Event inserted at beat %lld.", (long long)i);
        fail_unless(ret == events[i],
                "Column_get() returned Event %p instead of %p.",
                ret, events[i]);
    }
    fail_unless(Column_remove(col, events[0]),
            "Failed to remove Event 0.");
    Event* ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[1],
            "Column_get() returned Event %p instead of %p.", ret, events[1]);
    for (int64_t i = 2; i < 7; ++i)
    {
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p (beat %lld).",
                ret, events[i], (long long)i);
    }
    fail_unless(Column_remove(col, events[3]),
            "Failed to remove Event 3.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[1],
            "Column_get() returned Event %p instead of %p.", ret, events[1]);
    for (int64_t i = 2; i < 7; ++i)
    {
        if (i == 3)
        {
            continue;
        }
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p (beat %lld).",
                ret, events[i], (long long)i);
    }
    fail_unless(Column_remove(col, events[1]),
            "Failed to remove Event 1.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[2],
            "Column_get() returned Event %p instead of %p.", ret, events[2]);
    for (int64_t i = 4; i < 7; ++i)
    {
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p (beat %lld).",
                ret, events[i], (long long)i);
    }
    fail_unless(Column_remove(col, events[2]),
            "Failed to remove Event 2.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[4],
            "Column_get() returned Event %p instead of %p.", ret, events[4]);
    for (int64_t i = 5; i < 7; ++i)
    {
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p (beat %lld).",
                ret, events[i], (long long)i);
    }
    for (int i = 4; i < 7; ++i)
    {
        fail_unless(Column_remove(col, events[i]),
                "Failed to remove Event %d.", i);
    }
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == NULL,
            "Column_get() returned an Event from a supposedly empty Column.");
    
    for (int64_t i = 0; i < 7; ++i)
    {
        events[i] = new_Event(Reltime_set(RELTIME_AUTO, 1, 0), EVENT_TYPE_NOTE_ON);
        if (events[i] == NULL)
        {
            fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Column_ins(col, events[i]))
        {
            fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
            abort();
        }
        Event* ret = Column_iter_get(citer, Reltime_set(RELTIME_AUTO, 1, 0));
        fail_if(ret == NULL,
                "Couldn't find the Event inserted at beat %lld.", (long long)i);
        fail_unless(ret == events[0],
                "Column_get() returned Event %p instead of %p.",
                ret, events[0]);
    }
    fail_unless(Column_remove(col, events[1]),
            "Failed to remove Event 1.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[0],
            "Column_get() returned Event %p instead of %p.", ret, events[0]);
    for (int64_t i = 2; i < 7; ++i)
    {
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p.",
                ret, events[i]);
    }
    fail_unless(Column_remove(col, events[5]),
            "Failed to remove Event 5.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[0],
            "Column_get() returned Event %p instead of %p.", ret, events[0]);
    for (int64_t i = 2; i < 7; ++i)
    {
        if (i == 5)
        {
            continue;
        }
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p.",
                ret, events[i]);
    }
    fail_unless(Column_remove(col, events[0]),
            "Failed to remove Event 0.");
    fail_unless(Column_remove(col, events[6]),
            "Failed to remove Event 6.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[2],
            "Column_get() returned Event %p instead of %p.", ret, events[2]);
    for (int64_t i = 3; i < 5; ++i)
    {
        ret = Column_iter_get_next(citer);
        fail_unless(ret == events[i],
                "Column_get_next() returned Event %p instead of %p.",
                ret, events[i]);
    }
    fail_unless(Column_remove(col, events[3]),
            "Failed to remove Event 3.");
    ret = Column_iter_get(citer, Reltime_init(RELTIME_AUTO));
    fail_unless(ret == events[2],
            "Column_get() returned Event %p instead of %p.", ret, events[2]);
    ret = Column_iter_get_next(citer);
    fail_unless(ret == events[4],
            "Column_get_next() returned Event %p instead of %p.", ret, events[4]);
    del_Column_iter(citer);
    del_Column(col);
}
END_TEST

#ifndef NDEBUG
START_TEST (remove_break_col_null)
{
    Event* event = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (event == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        return;
    }
    Column_remove(NULL, event);
    del_Event(event);
}
END_TEST

START_TEST (remove_break_event_null)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_remove(col, NULL);
    del_Column(col);
}
END_TEST
#endif


START_TEST (clear)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Event* events[7] = { NULL };
    for (int64_t i = 0; i < 7; ++i)
    {
        events[i] = new_Event(Reltime_set(RELTIME_AUTO, i, 0), EVENT_TYPE_NOTE_ON);
        if (events[i] == NULL)
        {
            fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
            abort();
        }
        if (!Column_ins(col, events[i]))
        {
            fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
            abort();
        }
        fail_if(Column_iter_get(citer, Reltime_init(RELTIME_AUTO)) == NULL,
                "Column_get() returned NULL unexpectedly.");
    }
    Column_clear(col);
    fail_unless(Column_iter_get_next(citer) == NULL,
            "Column_get_next() returned an Event after Column_clear().");
    fail_unless(Column_iter_get(citer, Reltime_init(RELTIME_AUTO)) == NULL,
            "Column_get() returned an Event after Column_clear().");
    del_Column_iter(citer);
    del_Column(col);
}
END_TEST

#ifndef NDEBUG
START_TEST (clear_break_col_null)
{
    Column_clear(NULL);
}
END_TEST
#endif


START_TEST (length)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Reltime* len = Reltime_set(RELTIME_AUTO, 1, 2);
    Column_set_length(col, len);
    Reltime* ret = Column_length(col);
    fail_if(ret == len,
            "Column_set_length() stored the passed Reltime instead of copying.");
    fail_unless(Reltime_cmp(ret, len) == 0,
            "Column_length() returned a wrong time.");
    del_Column(col);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_length_break_col_null)
{
    Column_set_length(NULL, Reltime_init(RELTIME_AUTO));
}
END_TEST

START_TEST (set_length_break_len_null)
{
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_set_length(col, NULL);
    del_Column(col);
}
END_TEST

START_TEST (length_break_col_null)
{
    Column_length(NULL);
}
END_TEST
#endif


Suite* Column_suite(void)
{
    Suite* s = suite_create("Column");
    TCase* tc_new = tcase_create("new");
    TCase* tc_ins = tcase_create("ins");
    TCase* tc_remove = tcase_create("remove");
    TCase* tc_clear = tcase_create("clear");
    TCase* tc_length = tcase_create("length");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_ins);
    suite_add_tcase(s, tc_remove);
    suite_add_tcase(s, tc_clear);
    suite_add_tcase(s, tc_length);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_ins, timeout);
    tcase_set_timeout(tc_remove, timeout);
    tcase_set_timeout(tc_clear, timeout);
    tcase_set_timeout(tc_length, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_ins, ins);
    tcase_add_test(tc_remove, col_remove);
    tcase_add_test(tc_clear, clear);
    tcase_add_test(tc_length, length);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_ins, ins_break_col_null, SIGABRT);
    tcase_add_test_raise_signal(tc_ins, ins_break_event_null, SIGABRT);

    tcase_add_test_raise_signal(tc_remove, remove_break_col_null, SIGABRT);
    tcase_add_test_raise_signal(tc_remove, remove_break_event_null, SIGABRT);

    tcase_add_test_raise_signal(tc_clear, clear_break_col_null, SIGABRT);

    tcase_add_test_raise_signal(tc_length, set_length_break_col_null, SIGABRT);
    tcase_add_test_raise_signal(tc_length, set_length_break_len_null, SIGABRT);
    tcase_add_test_raise_signal(tc_length, length_break_col_null, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Column_suite();
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


