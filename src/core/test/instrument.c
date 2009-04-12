

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

#include <frame_t.h>
#include <Voice_state.h>
#include <Generator_debug.h>
#include <Instrument.h>


Suite* Instrument_suite(void);


START_TEST (new)
{
    frame_t buf_l[100] = { 0 };
    frame_t buf_r[100] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 100, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    del_Instrument(ins);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_bufs_null)
{
    Instrument* ins = new_Instrument(NULL, 1, 1);
    del_Instrument(ins);
}
END_TEST

START_TEST (new_break_buf_len_inv)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 0, 1);
    del_Instrument(ins);
}
END_TEST

START_TEST (new_break_events_inv)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 1, 0);
    del_Instrument(ins);
}
END_TEST
#endif


START_TEST (mix)
{
    frame_t buf_l[128] = { 0 };
    frame_t buf_r[128] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 128, 16);
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
    Voice_state state;
    Voice_state_init(&state, NULL);
    state.freq = 16;
    Instrument_mix(ins, &state, 128, 0, 64);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 100; ++i)
    {
        if (i < 40)
        {
            if (i % 4 == 0)
            {
                fail_unless(bufs[0][i] > 0.99,
                        "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
            }
            else
            {
                fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                        "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
            }
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 16;
    for (int i = 0; i < 121; i += 7)
    {
        if (i < 40)
        {
            fail_unless(state.active,
                    "Instrument became inactive prematurely (after sample %d).", i);
        }
        else
        {
            fail_unless(!state.active,
                    "Instrument didn't become inactive after finishing mixing (at sample %d).", i);
            break;
        }
        Instrument_mix(ins, &state, i + 7, i, 64);
    }
    for (int i = 0; i < 128; ++i)
    {
        if (i < 40)
        {
            if (i % 4 == 0)
            {
                fail_unless(bufs[0][i] > 0.99,
                        "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
            }
            else
            {
                fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                        "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
            }
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 16;
    for (int i = 0; i < 127; ++i)
    {
        if (i < 40)
        {
            fail_unless(state.active,
                    "Instrument became inactive prematurely (after sample %d).", i);
        }
        else
        {
            fail_unless(!state.active,
                    "Instrument didn't become inactive after finishing mixing (at sample %d).", i);
            break;
        }
        Instrument_mix(ins, &state, i + 1, i, 64);
    }
    for (int i = 0; i < 128; ++i)
    {
        if (i < 40)
        {
            if (i % 4 == 0)
            {
                fail_unless(bufs[0][i] > 0.99,
                        "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
            }
            else
            {
                fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                        "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
            }
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    
    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 16;
    Instrument_mix(ins, &state, 20, 0, 64);
    state.note_on = false;
    Instrument_mix(ins, &state, 128, 20, 64);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 20; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99,
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
            fail_unless(bufs[0][i] < -0.99,
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
    
    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 16;
    Instrument_mix(ins, &state, 36, 0, 64);
    state.note_on = false;
    Instrument_mix(ins, &state, 128, 36, 64);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 36; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 36; i < 40; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] < -0.99,
                    "Buffer contains %f at index %d (expected -1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
    }
    for (int i = 40; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 1;
        buf_r[i] = 1;
    }
    state.freq = 16;
    Instrument_mix(ins, &state, 36, 0, 64);
    state.note_on = false;
    Instrument_mix(ins, &state, 128, 36, 64);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 36; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 1.99,
                    "Buffer contains %f at index %d (expected 2).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                    "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
        }
    }
    for (int i = 36; i < 40; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 40; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 1.01,
                "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
    }

    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 8;
    Instrument_mix(ins, &state, 128, 0, 64);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 80; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] > 0.99,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 80; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }

    Voice_state_init(&state, NULL);
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = 0;
        buf_r[i] = 0;
    }
    state.freq = 8;
    Instrument_mix(ins, &state, 128, 0, 32);
    fail_unless(!state.active,
            "Instrument didn't become inactive after finishing mixing.");
    for (int i = 0; i < 40; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99,
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

    del_Instrument(ins);
}
END_TEST

#ifndef NDEBUG
START_TEST (mix_break_ins_null)
{
    Voice_state state;
    Voice_state_init(&state, NULL);
    Instrument_mix(NULL, &state, 0, 0, 1);
}
END_TEST

START_TEST (mix_break_state_null)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 1, 1);
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
    Instrument_mix(ins, NULL, 0, 0, 1);
    del_Instrument(ins);
}
END_TEST

// XXX: enable again after implementing instrument buffers
#if 0
START_TEST (mix_break_nframes_inv)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 1, 1);
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
    Voice_state state;
    Voice_state_init(&state, NULL);
    Instrument_mix(ins, &state, 2, 0, 1);
    del_Instrument(ins);
}
END_TEST
#endif

START_TEST (mix_break_freq_inv)
{
    frame_t buf_l[1] = { 0 };
    frame_t buf_r[1] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, 1, 1);
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
    Voice_state state;
    Voice_state_init(&state, NULL);
    Instrument_mix(ins, &state, 1, 0, 0);
    del_Instrument(ins);
}
END_TEST
#endif


Suite* Instrument_suite(void)
{
    Suite* s = suite_create("Instrument");
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
    tcase_add_test_raise_signal(tc_new, new_break_bufs_null, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break_buf_len_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break_events_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_mix, mix_break_ins_null, SIGABRT);
    tcase_add_test_raise_signal(tc_mix, mix_break_state_null, SIGABRT);
//  tcase_add_test_raise_signal(tc_mix, mix_break_nframes_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_mix, mix_break_freq_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Instrument_suite();
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


