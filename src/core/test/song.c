

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
#include <math.h>

#include <check.h>

#include <Real.h>
#include <Note_table.h>
#include <Reltime.h>
#include <Event.h>
#include <Instrument.h>
#include <Voice.h>
#include <Voice_pool.h>
#include <Column.h>
#include <Channel.h>
#include <Pattern.h>
#include <Song.h>

#include <xmemory.h>


Suite* Song_suite(void);

Playdata* init_play(Song* song);


Playdata* init_play(Song* song)
{
	if (song == NULL)
	{
		fprintf(stderr, "Test used init_play() incorrectly\n");
		return NULL;
	}
	Playdata* play = xalloc(Playdata);
	if (play == NULL)
	{
		fprintf(stderr, "xalloc() returned NULL -- out of memory?\n");
		return NULL;
	}
	play->voice_pool = new_Voice_pool(16, 16);
	if (play->voice_pool == NULL)
	{
		fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
		return NULL;
	}
	for (int i = 0; i < PAT_CHANNELS; ++i)
	{
		play->channels[i] = new_Channel(song->insts);
		if (play->channels[i] == NULL)
		{
			fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
			return NULL;
		}
	}
	play->play = STOP;
	play->freq = 0;
	Reltime_init(&play->play_time);
	play->tempo = 0;
	Reltime_init(&play->pos);
	play->order = song->order;
	return play;
}


START_TEST (new)
{
	Song* song = new_Song(2, 128);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	Playdata* play = init_play(song);
	if (play == NULL) abort();
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_buf_count_inv1)
{
	new_Song(0, 1);
}
END_TEST

START_TEST (new_break_buf_count_inv2)
{
	new_Song(BUF_COUNT_MAX + 1, 1);
}
END_TEST

START_TEST (new_break_buf_size_inv)
{
	new_Song(1, 0);
}
END_TEST
#endif


Suite* Song_suite(void)
{
	Suite* s = suite_create("Song");
	TCase* tc_new = tcase_create("new");
	suite_add_tcase(s, tc_new);

	tcase_add_test(tc_new, new);

#ifndef NDEBUG
	tcase_add_test_raise_signal(tc_new, new_break_buf_count_inv1, SIGABRT);
	tcase_add_test_raise_signal(tc_new, new_break_buf_count_inv2, SIGABRT);
	tcase_add_test_raise_signal(tc_new, new_break_buf_size_inv, SIGABRT);
#endif

	return s;
}


int main(void)
{
	int fail_count = 0;
	Suite* s = Song_suite();
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


