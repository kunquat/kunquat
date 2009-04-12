

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
#include <math.h>

#include <check.h>

#include <Real.h>
#include <Note_table.h>
#include <Reltime.h>
#include <Event.h>
#include <Generator_debug.h>
#include <Instrument.h>
#include <Voice.h>


Suite* Voice_suite(void);


START_TEST (new)
{
    Voice* voice = new_Voice(1);
    if (voice == NULL)
    {
        fprintf(stderr, "new_Voice() returned NULL -- out of memory?\n");
        abort();
    }
    fail_unless(voice->prio == VOICE_PRIO_INACTIVE,
            "new_Voice() set priority to %d instead of VOICE_PRIO_INACTIVE.", voice->prio);
    del_Voice(voice);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_events_inv)
{
    new_Voice(0);
}
END_TEST
#endif


START_TEST (mix)
{
    // Testing scenario 1:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note frequency is 2 Hz (2 cycles/beat).
    // Note starts at the beginning and plays until the end
    // Result should be (1, 0.5, 0.5, 0.5) 10 times, the rest are zero.
    frame_t buf_l[128] = { 0 };
    frame_t buf_r[128] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 128, 2);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Generator_debug* gen_debug = new_Generator_debug(Instrument_get_params(ins));
    if (gen_debug == NULL)
    {
        fprintf(stderr, "new_Generator_debug() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument_set_gen(ins, 0, (Generator*)gen_debug);
    Note_table* notes = new_Note_table(L"test", 2, Real_init_as_frac(REAL_AUTO, 2, 1));
    if (notes == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- out of memory?\n");
        abort();
    }
    Note_table_set_note(notes, 0, L"=", Real_init(REAL_AUTO));
    Instrument_set_note_table(ins, &notes);
    Voice* voice = new_Voice(2);
    if (voice == NULL)
    {
        fprintf(stderr, "new_Voice() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev_on = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (ev_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event_set_int(ev_on, 0, 0);
    Event_set_int(ev_on, 1, -1);
    Event_set_int(ev_on, 2, NOTE_TABLE_MIDDLE_OCTAVE);
    Event* ev_off = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_OFF);
    if (ev_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Voice_init(voice, ins);
    fail_unless(voice->prio == VOICE_PRIO_NEW,
            "Voice_init() set Voice priority to %d (expected VOICE_PRIO_NEW).", voice->prio);
    fail_unless(Voice_add_event(voice, ev_on, 0),
            "Voice_add_event() failed.");
    Voice_mix(voice, 128, 0, 8);
    fail_unless(voice->prio == VOICE_PRIO_INACTIVE,
            "Voice priority is %d after finishing mixing (expected VOICE_PRIO_INACTIVE).", voice->prio);
    for (int i = 0; i < 40; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 40; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 2:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note frequency is 2 Hz (2 cycles/beat).
    // Note starts at the beginning.
    // Note Off event is located at frame 20.
    // Result should be (1, 0.5, 0.5, 0.5) 5 times, (-1, -0.5, -0.5, -0.5) twice, the rest are zero.
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Voice_init(voice, ins);
    fail_unless(voice->prio == VOICE_PRIO_NEW,
            "Voice_init() set Voice priority to %d (expected VOICE_PRIO_NEW).", voice->prio);
    fail_unless(Voice_add_event(voice, ev_on, 0),
            "Voice_add_event() failed.");
    fail_unless(Voice_add_event(voice, ev_off, 20),
            "Voice_add_event() failed.");
    Voice_mix(voice, 20, 0, 8);
    fail_unless(voice->prio == VOICE_PRIO_BG,
            "Voice priority is %d after reaching Note Off (expected VOICE_PRIO_BG).", voice->prio);
    Voice_mix(voice, 128, 20, 8);
    fail_unless(voice->prio == VOICE_PRIO_INACTIVE,
            "Voice priority is %d after finishing mixing (expected VOICE_PRIO_INACTIVE).", voice->prio);
    for (int i = 0; i < 20; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 20; i < 28; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] < -0.99 && bufs[0][i] > -1.01,
                    "Buffer contains %f at index %d (expected -1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
    }
    for (int i = 28; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 3:
    //
    // Mixing frequency is 16 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note frequency is 2 Hz (2 cycles/beat).
    // Note starts at the beginning.
    // Note Off event is located at frame 70.
    // Result should be (1, (0.5 7 times)) 8 times,
    //    (1, 0.5, 0.5, 0.5, 0.5, 0.5) once,
    //    (-0.5, -0.5, -1, (-0.5 7 times)) once, the rest are zero.
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Voice_init(voice, ins);
    fail_unless(voice->prio == VOICE_PRIO_NEW,
            "Voice_init() set Voice priority to %d (expected VOICE_PRIO_NEW).", voice->prio);
    fail_unless(Voice_add_event(voice, ev_on, 0),
            "Voice_add_event() failed.");
    fail_unless(Voice_add_event(voice, ev_off, 70),
            "Voice_add_event() failed.");
    Voice_mix(voice, 128, 0, 16);
    fail_unless(voice->prio == VOICE_PRIO_INACTIVE,
            "Voice priority is %d after finishing mixing (expected VOICE_PRIO_INACTIVE).", voice->prio);
    for (int i = 0; i < 70; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 70; i < 80; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.99 && bufs[0][i] > -1.01,
                    "Buffer contains %f at index %d (expected -1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
    }
    for (int i = 80; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    del_Event(ev_off);
    del_Event(ev_on);
    del_Voice(voice);
    del_Note_table(notes);
    del_Instrument(ins);
}
END_TEST

#ifndef NDEBUG
START_TEST (mix_break_voice_null)
{
    Voice_mix(NULL, 1, 0, 1);
}
END_TEST

START_TEST (mix_break_freq_inv)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 1, 2);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        return;
    }
    Generator_debug* gen_debug = new_Generator_debug(Instrument_get_params(ins));
    if (gen_debug == NULL)
    {
        fprintf(stderr, "new_Generator_debug() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument_set_gen(ins, 0, (Generator*)gen_debug);
    Note_table* notes = new_Note_table(L"test", 2, Real_init_as_frac(REAL_AUTO, 2, 1));
    if (notes == NULL)
    {
        fprintf(stderr, "new_Note_table() returned NULL -- out of memory?\n");
        return;
    }
    Instrument_set_note_table(ins, &notes);
    Voice* voice = new_Voice(1);
    if (voice == NULL)
    {
        fprintf(stderr, "new_Voice() returned NULL -- out of memory?\n");
        return;
    }
    Event* ev_on = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (ev_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        return;
    }
    fail_unless(Voice_add_event(voice, ev_on, 0),
            "Voice_add_event() failed.");
    Voice_mix(voice, 1, 0, 0);
    del_Voice(voice);
}
END_TEST
#endif


Suite* Voice_suite(void)
{
    Suite* s = suite_create("Voice");
    TCase* tc_new = tcase_create("new");
    TCase* tc_mix = tcase_create("mix");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_mix);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_mix, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_mix, mix);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break_events_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_mix, mix_break_voice_null, SIGABRT);
    tcase_add_test_raise_signal(tc_mix, mix_break_freq_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Voice_suite();
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


