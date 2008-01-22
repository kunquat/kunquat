

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
#include <wchar.h>

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
	Song* song = new_Song(2, 128, 64);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	fail_if(Song_get_name(song) == NULL,
			"new_Song() created a Song without a name.");
	double tempo = Song_get_tempo(song);
	fail_unless(isfinite(tempo) && tempo > 0,
			"new_Song() created a Song without a sane initial tempo (%lf).", tempo);
	double mix_vol = Song_get_mix_vol(song);
	fail_unless(isfinite(mix_vol),
			"new_Song() created a Song without a sane initial mixing volume (%lf).", mix_vol);
	double global_vol = Song_get_global_vol(song);
	fail_unless(isfinite(global_vol),
			"new_Song() created a Song without a sane initial global volume (%lf).", global_vol);
	int buf_count = Song_get_buf_count(song);
	fail_unless(buf_count == 2,
			"new_Song() created a Song with a wrong amount of buffers (%d).", buf_count);
	frame_t** bufs = Song_get_bufs(song);
	fail_if(bufs == NULL,
			"new_Song() created a Song without buffers.");
	fail_if(bufs[0] == NULL,
			"new_Song() created a Song without buffers.");
	fail_if(bufs[1] == NULL,
			"new_Song() created a Song without a second buffer.");
	Order* order = Song_get_order(song);
	fail_if(order == NULL,
			"new_Song() created a Song without Order list.");
	Pat_table* pats = Song_get_pats(song);
	fail_if(pats == NULL,
			"new_Song() created a Song without Pattern table.");
	Ins_table* insts = Song_get_insts(song);
	fail_if(insts == NULL,
			"new_Song() created a Song without Instrument table.");
	Note_table* notes = Song_get_notes(song);
	fail_if(notes == NULL,
			"new_Song() created a Song without Note table.");
	Event_queue* events = Song_get_events(song);
	fail_if(events == NULL,
			"new_Song() created a Song without Event queue.");
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_buf_count_inv1)
{
	new_Song(0, 1, 1);
}
END_TEST

START_TEST (new_break_buf_count_inv2)
{
	new_Song(BUF_COUNT_MAX + 1, 1, 1);
}
END_TEST

START_TEST (new_break_buf_size_inv)
{
	new_Song(1, 0, 1);
}
END_TEST

START_TEST (new_break_events_inv)
{
	new_Song(1, 1, 0);
}
END_TEST
#endif


START_TEST (set_get_name)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	wchar_t name[132] = L"Name";
	Song_set_name(song, name);
	wchar_t* ret = Song_get_name(song);
	fail_if(ret == NULL,
			"Song_get_name() returned NULL.");
	fail_if(name == ret,
			"Song_set_name() copied the reference instead of the characters.");
	fail_unless(wcscmp(name, ret) == 0,
			"Song_set_name() copied the name incorrectly.");
	for (int i = 4; i < 132; ++i)
	{
		name[i] = L'!';
	}
	Song_set_name(song, name);
	ret = Song_get_name(song);
	fail_if(ret == NULL,
			"Song_get_name() returned NULL.");
	fail_if(name == ret,
			"Song_set_name() copied the reference instead of the characters.");
	fail_unless(ret[127] == L'\0',
			"Song_set_name() didn't truncate the name correctly.");
	fail_unless(wcsncmp(name, ret, 127) == 0,
			"Song_set_name() copied the name incorrectly.");
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_name_break_song_null)
{
	Song_set_name(NULL, L"Oops");
}
END_TEST

START_TEST (set_name_break_name_null)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_name(song, NULL);
	del_Song(song);
}
END_TEST

START_TEST (get_name_break_song_null)
{
	Song_get_name(NULL);
}
END_TEST
#endif


START_TEST (set_get_tempo)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	Song_set_tempo(song, 16);
	double ret = Song_get_tempo(song);
	fail_unless(ret, 16,
			"Song_get_tempo() returned %lf instead of %lf.", ret, 16);
	Song_set_tempo(song, 120);
	ret = Song_get_tempo(song);
	fail_unless(ret, 120,
			"Song_get_tempo() returned %lf instead of %lf.", ret, 120);
	Song_set_tempo(song, 512);
	ret = Song_get_tempo(song);
	fail_unless(ret, 512,
			"Song_get_tempo() returned %lf instead of %lf.", ret, 512);
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_tempo_break_song_null)
{
	Song_set_tempo(NULL, 120);
}
END_TEST

START_TEST (set_tempo_break_tempo_inv1)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_tempo(song, 0);
	del_Song(song);
}
END_TEST

START_TEST (set_tempo_break_tempo_inv2)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_tempo(song, -INFINITY);
	del_Song(song);
}
END_TEST

START_TEST (set_tempo_break_tempo_inv3)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_tempo(song, INFINITY);
	del_Song(song);
}
END_TEST

START_TEST (set_tempo_break_tempo_inv4)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_tempo(song, NAN);
	del_Song(song);
}
END_TEST

START_TEST (get_tempo_break_song_null)
{
	Song_get_tempo(NULL);
}
END_TEST
#endif


START_TEST (set_get_mix_vol)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	Song_set_mix_vol(song, -INFINITY);
	double ret = Song_get_mix_vol(song);
	fail_unless(ret == -INFINITY,
			"Song_get_mix_vol() returned %lf instead of %lf.", ret, -INFINITY);
	Song_set_mix_vol(song, 0);
	ret = Song_get_mix_vol(song);
	fail_unless(ret == 0,
			"Song_get_mix_vol() returned %lf instead of %lf.", ret, 0);
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_mix_vol_break_song_null)
{
	Song_set_mix_vol(NULL, 0);
}
END_TEST

START_TEST (set_mix_vol_break_mix_vol_inv1)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_mix_vol(song, INFINITY);
	del_Song(song);
}
END_TEST

START_TEST (set_mix_vol_break_mix_vol_inv2)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_mix_vol(song, NAN);
	del_Song(song);
}
END_TEST

START_TEST (get_mix_vol_break_song_null)
{
	Song_get_mix_vol(NULL);
}
END_TEST
#endif


START_TEST (set_get_global_vol)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		abort();
	}
	Song_set_global_vol(song, -INFINITY);
	double ret = Song_get_global_vol(song);
	fail_unless(ret == -INFINITY,
			"Song_get_global_vol() returned %lf instead of %lf.", ret, -INFINITY);
	Song_set_global_vol(song, 0);
	ret = Song_get_global_vol(song);
	fail_unless(ret == 0,
			"Song_get_global_vol() returned %lf instead of %lf.", ret, 0);
	del_Song(song);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_global_vol_break_song_null)
{
	Song_set_global_vol(NULL, 0);
}
END_TEST

START_TEST (set_global_vol_break_global_vol_inv1)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_global_vol(song, INFINITY);
	del_Song(song);
}
END_TEST

START_TEST (set_global_vol_break_global_vol_inv2)
{
	Song* song = new_Song(1, 1, 1);
	if (song == NULL)
	{
		fprintf(stderr, "new_Song() returned NULL -- out of memory?\n");
		return;
	}
	Song_set_global_vol(song, NAN);
	del_Song(song);
}
END_TEST

START_TEST (get_global_vol_break_song_null)
{
	Song_get_global_vol(NULL);
}
END_TEST
#endif


Suite* Song_suite(void)
{
	Suite* s = suite_create("Song");
	TCase* tc_new = tcase_create("new");
	TCase* tc_set_get_name = tcase_create("set_get_name");
	TCase* tc_set_get_tempo = tcase_create("set_get_tempo");
	TCase* tc_set_get_mix_vol = tcase_create("set_get_mix_vol");
	TCase* tc_set_get_global_vol = tcase_create("set_get_global_vol");
	suite_add_tcase(s, tc_new);
	suite_add_tcase(s, tc_set_get_name);
	suite_add_tcase(s, tc_set_get_tempo);
	suite_add_tcase(s, tc_set_get_mix_vol);
	suite_add_tcase(s, tc_set_get_global_vol);

	tcase_add_test(tc_new, new);
	tcase_add_test(tc_set_get_name, set_get_name);
	tcase_add_test(tc_set_get_tempo, set_get_tempo);
	tcase_add_test(tc_set_get_mix_vol, set_get_mix_vol);
	tcase_add_test(tc_set_get_global_vol, set_get_global_vol);

#ifndef NDEBUG
	tcase_add_test_raise_signal(tc_new, new_break_buf_count_inv1, SIGABRT);
	tcase_add_test_raise_signal(tc_new, new_break_buf_count_inv2, SIGABRT);
	tcase_add_test_raise_signal(tc_new, new_break_buf_size_inv, SIGABRT);
	tcase_add_test_raise_signal(tc_new, new_break_events_inv, SIGABRT);

	tcase_add_test_raise_signal(tc_set_get_name, set_name_break_song_null, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_name, set_name_break_name_null, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_name, get_name_break_song_null, SIGABRT);

	tcase_add_test_raise_signal(tc_set_get_tempo, set_tempo_break_song_null, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_tempo, set_tempo_break_tempo_inv1, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_tempo, set_tempo_break_tempo_inv2, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_tempo, set_tempo_break_tempo_inv3, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_tempo, set_tempo_break_tempo_inv4, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_tempo, get_tempo_break_song_null, SIGABRT);

	tcase_add_test_raise_signal(tc_set_get_mix_vol, set_mix_vol_break_song_null, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_mix_vol, set_mix_vol_break_mix_vol_inv1, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_mix_vol, set_mix_vol_break_mix_vol_inv2, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_mix_vol, get_mix_vol_break_song_null, SIGABRT);

	tcase_add_test_raise_signal(tc_set_get_global_vol, set_global_vol_break_song_null, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_global_vol, set_global_vol_break_global_vol_inv1, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_global_vol, set_global_vol_break_global_vol_inv2, SIGABRT);
	tcase_add_test_raise_signal(tc_set_get_global_vol, get_global_vol_break_song_null, SIGABRT);
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


