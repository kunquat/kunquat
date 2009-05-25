

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
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <float.h>

#include <check.h>

#include <Event.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <Reltime.h>


Suite* Event_suite(void);


START_TEST (new)
{
    Reltime* r = Reltime_init(RELTIME_AUTO);
    Event* event = new_Event_voice_note_on(r);
    if (event == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    fail_if(r == Event_get_pos(event),
            "new_Event() didn't copy the position.");
    fail_unless(Reltime_cmp(r, Event_get_pos(event)) == 0,
            "new_Event() set the position incorrectly.");
    fail_unless(Event_get_type(event) == EVENT_TYPE_NOTE_ON,
            "new_Event() set the Event type to %d instead of EVENT_TYPE_NOTE_ON.", Event_get_type(event));
    del_Event(event);
}
END_TEST


Suite* Event_suite(void)
{
    Suite* s = suite_create("Event");
    TCase* tc_new = tcase_create("new");
    suite_add_tcase(s, tc_new);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);

    tcase_add_test(tc_new, new);

#ifndef NDEBUG
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Event_suite();
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


