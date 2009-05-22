

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
#include <signal.h>

#include <check.h>

#include <Voice_state.h>


Suite* Voice_state_suite(void);


START_TEST (init)
{
    Voice_state state;
    Voice_state* ret = Voice_state_init(&state, NULL);
    fail_unless(ret == &state,
            "Voice_state_init() returned %p instead of %p.", ret, &state);
    fail_unless(state.active,
            "Voice_state_init() didn't set active to true.");
    fail_unless(state.pos == 0,
            "Voice_state_init() set pos to %llu instead of 0.", (unsigned long long)state.pos);
    fail_unless(state.pos_rem == 0,
            "Voice_state_init() set pos_rem to %lf instead of 0.", state.pos_rem);
    fail_unless(state.rel_pos == 0,
            "Voice_state_init() set rel_pos to %llu instead of 0.", (unsigned long long)state.rel_pos);
    fail_unless(state.rel_pos_rem == 0,
            "Voice_state_init() set rel_pos_rem to %lf instead of 0.", state.rel_pos_rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (init_break_state_null)
{
    Voice_state_init(NULL, NULL);
}
END_TEST
#endif


Suite* Voice_state_suite(void)
{
    Suite* s = suite_create("Voice_state");
    TCase* tc_init = tcase_create("init");
    suite_add_tcase(s, tc_init);

    int timeout = 10;
    tcase_set_timeout(tc_init, timeout);

    tcase_add_test(tc_init, init);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_init, init_break_state_null, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Voice_state_suite();
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


