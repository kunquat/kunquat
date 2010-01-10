

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
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
    fail_unless(Event_get_type(event) == EVENT_VOICE_NOTE_ON,
            "new_Event() set the Event type to %d instead of EVENT_VOICE_NOTE_ON.", Event_get_type(event));
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


