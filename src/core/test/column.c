

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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
	fail_unless(Column_get_next(col) == NULL,
			"new_Column() created a non-empty Column.");
	fail_unless(Column_get(col, Reltime_init(RELTIME_AUTO)) == NULL,
			"new_Column() created a non-empty Column.");
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
		Event* ret = Column_get(col, Reltime_set(RELTIME_AUTO, i, 0));
		fail_if(ret == NULL,
				"Couldn't find the Event inserted at beat %lld.", (long long)i);
		int64_t beat = Event_pos(ret)->beats;
		fail_unless(beat == i,
				"Column_get() returned Event %lld instead of %lld.",
				(long long)beat, (long long)i);
		for (int64_t k = i + 1; k <= 4; ++k)
		{
			ret = Column_get_next(col);
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
	Event* ret = Column_get(col, Reltime_set(RELTIME_AUTO, 2, 0));
	fail_if(ret == NULL,
			"Couldn't find the Event inserted at beat 2.");
	fail_if(ret == event,
			"Column_get() returned the wrong Event at beat 2.");
	int64_t beat = Event_pos(ret)->beats;
	fail_unless(beat == 2,
			"Column_get() returned Event %lld instead of 2.", (long long)beat);
	ret = Column_get_next(col);
	fail_if(ret == NULL,
			"Couldn't find the Event inserted at beat 2.");
	fail_unless(ret == event,
			"Column_get() returned the wrong Event at beat 2.");
	beat = Event_pos(ret)->beats;
	fail_unless(beat == 2,
			"Column_get() returned Event %lld instead of 2.", (long long)beat);
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
	fail_if(Column_remove(col, events[0]),
			"Duplicate removal succeeded.");
	fail_unless(Column_get(col, Reltime_set(RELTIME_AUTO, 0, 0)) == NULL,
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
		Event* ret = Column_get(col, Reltime_set(RELTIME_AUTO, i, 0));
		fail_if(ret == NULL,
				"Couldn't find the Event inserted at beat %lld.", (long long)i);
		fail_unless(ret == events[i],
				"Column_get() returned Event %p instead of %p.",
				ret, events[i]);
	}
	fail_unless(Column_remove(col, events[0]),
			"Failed to remove Event 0.");
	fail_if(Column_remove(col, events[0]),
			"Duplicate removal of Event 0 succeeded.");
	Event* ret = Column_get(col, Reltime_init(RELTIME_AUTO));
	fail_unless(ret == events[1],
			"Column_get() returned Event %p instead of %p.", ret, events[1]);
	for (int64_t i = 2; i < 7; ++i)
	{
		ret = Column_get_next(col);
		fail_unless(ret == events[i],
				"Column_get_next() returned Event %p instead of %p (beat %lld).",
				ret, events[i], (long long)i);
	}
	fail_unless(Column_remove(col, events[3]),
			"Failed to remove Event 3.");
	fail_if(Column_remove(col, events[3]),
			"Duplicate removal of Event 3 succeeded.");
	ret = Column_get(col, Reltime_init(RELTIME_AUTO));
	fail_unless(ret == events[1],
			"Column_get() returned Event %p instead of %p.", ret, events[1]);
	for (int64_t i = 2; i < 7; ++i)
	{
		if (i == 3)
		{
			continue;
		}
		ret = Column_get_next(col);
		fail_unless(ret == events[i],
				"Column_get_next() returned Event %p instead of %p (beat %lld).",
				ret, events[i], (long long)i);
	}
	fail_unless(Column_remove(col, events[1]),
			"Failed to remove Event 1.");
	fail_if(Column_remove(col, events[1]),
			"Duplicate removal of Event 1 succeeded.");
	ret = Column_get(col, Reltime_init(RELTIME_AUTO));
	fail_unless(ret == events[2],
			"Column_get() returned Event %p instead of %p.", ret, events[2]);
	for (int64_t i = 4; i < 7; ++i)
	{
		ret = Column_get_next(col);
		fail_unless(ret == events[i],
				"Column_get_next() returned Event %p instead of %p (beat %lld).",
				ret, events[i], (long long)i);
	}
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


Suite* Column_suite(void)
{
	Suite* s = suite_create("Column");
	TCase* tc_new = tcase_create("new");
	TCase* tc_ins = tcase_create("ins");
	TCase* tc_remove = tcase_create("remove");
	suite_add_tcase(s, tc_new);
	suite_add_tcase(s, tc_ins);
	suite_add_tcase(s, tc_remove);

	tcase_add_test(tc_new, new);
	tcase_add_test(tc_ins, ins);
	tcase_add_test(tc_remove, col_remove);

#ifndef NDEBUG
	tcase_add_test_raise_signal(tc_ins, ins_break_col_null, SIGABRT);
	tcase_add_test_raise_signal(tc_ins, ins_break_event_null, SIGABRT);

	tcase_add_test_raise_signal(tc_remove, remove_break_col_null, SIGABRT);
	tcase_add_test_raise_signal(tc_remove, remove_break_event_null, SIGABRT);
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


