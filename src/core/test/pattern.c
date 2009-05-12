

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
#include <Voice_pool.h>
#include <Column.h>
#include <Channel.h>
#include <Pattern.h>

#include <xmemory.h>


Suite* Pattern_suite(void);

Playdata* init_play(void);


Playdata* init_play(void)
{
    Voice_pool* voice_pool = new_Voice_pool(16, 16);
    if (voice_pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return NULL;
    }
    Ins_table* insts = new_Ins_table(1);
    if (insts == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return NULL;
    }
    Playdata* play = new_Playdata(1, voice_pool, insts);
    if (play == NULL)
    {
        fprintf(stderr, "xalloc() returned NULL -- out of memory?\n");
        return NULL;
    }
    play->events = new_Event_queue(16);
    if (play->events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        return NULL;
    }
    play->mode = STOP;
    play->freq = 0;
    Reltime_init(&play->play_time);
    play->tempo = 0;
    Reltime_init(&play->pos);
    return play;
}


START_TEST (new)
{
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    Column* glob = Pattern_global(pat);
    fail_if(glob == NULL,
            "new_Pattern() didn't create a global Column.");
    Column* cols[COLUMNS_MAX] = { NULL };
    for (int i = 0; i < COLUMNS_MAX; ++i)
    {
        cols[i] = Pattern_col(pat, i);
        fail_if(cols[i] == NULL,
                "new_Pattern() didn't create Column #%d.", i);
        fail_if(cols[i] == glob,
                "Column #%d is the same as the global Column.", i);
    }
    del_Pattern(pat);
}
END_TEST


START_TEST (mix)
{
    Playdata* play = init_play();
    if (play == NULL) abort();
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        abort();
    }
    frame_t buf_l[256] = { 0 };
    frame_t buf_r[256] = { 0 };
    frame_t* bufs[2] = { buf_l, buf_r };
    Instrument* ins = new_Instrument(bufs, bufs, 128, 16);
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
    if (!Ins_table_set(play->channels[0]->insts, 1, ins))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    Event* ev1_on = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (ev1_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev1_off = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_OFF);
    if (ev1_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_on = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (ev2_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_off = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_OFF);
    if (ev2_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev3_on = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_NOTE_ON);
    if (ev3_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* evg_tempo = new_Event(Reltime_init(RELTIME_AUTO), EVENT_TYPE_GLOBAL_TEMPO);
    
    // Testing scenario 1:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note frequency is 2 Hz (2 cycles/beat).
    // Note starts at the beginning and plays until the end
    // Result should be (1, 0.5, 0.5, 0.5) 10 times, the rest are zero.
    play->mode = PLAY_PATTERN;
    play->freq = 8;
    play->tempo = 60;
    Reltime_init(&play->pos);
    Event_set_int(ev1_on, 0, 0);
    Event_set_int(ev1_on, 1, -1);
    Event_set_int(ev1_on, 2, NOTE_TABLE_MIDDLE_OCTAVE);
    Event_set_int(ev1_on, 3, 1);
    Column* col = Pattern_col(pat, 0);
    if (!Column_ins(col, ev1_on))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    uint32_t ret = Pattern_mix(pat, 256, 0, play);
    fail_unless(ret == 128,
            "Pattern_mix() mixed %lu frames instead of 128.", (unsigned long)ret);
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
    for (int i = 40; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    // Test again in small parts
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    for (int i = 0; i < 256; ++i)
    {
        if (Pattern_mix(pat, i + 1, i, play) < 1)
        {
            fail_unless(i == 128,
                    "Pattern_mix() reached the end in %d samples instead of 128.", i);
            break;
        }
        fail_if(i == 128,
                "Pattern_mix() didn't reach the end in 128 samples.");
    }
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
    for (int i = 40; i < 256; ++i)
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
    // Note starts at the beginning and plays until the end
    // A tempo change event is located at beat 2.
    // Result should be (1, 0.5, 0.5, 0.5) 10 times, the rest are zero.
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    Event_set_float(evg_tempo, 0, 120);
    Event_set_pos(evg_tempo, Reltime_set(RELTIME_AUTO, 2, 0));
    col = Pattern_global(pat);
    if (!Column_ins(col, evg_tempo))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    ret = Pattern_mix(pat, 256, 0, play);
    fail_unless(ret == 72,
            "Pattern_mix() mixed %lu frames instead of 72.", (unsigned long)ret);
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
    for (int i = 40; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    // Test again in small parts
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    fail_unless(play->tempo - 120 < 0.01,
            "Pattern_mix() didn't adjust the tempo.");
    play->tempo = 60;
    for (int i = 0; i < 256; ++i)
    {
        if (Pattern_mix(pat, i + 1, i, play) < 1)
        {
            fail_unless(i == 72,
                    "Pattern_mix() reached the end in %d samples instead of 72.", i);
            break;
        }
        fail_if(i == 72,
                "Pattern_mix() didn't reach the end in 72 samples.");
    }
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
    for (int i = 40; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 3:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note #1 frequency is 1 Hz (1 cycle/beat).
    // Note #1 starts at the beginning and is released at frame 2
    // Note #2 frequency is 2 Hz (2 cycles/beat).
    // Note #2 starts at position 0:(1/4) (frame 2) and plays until the end
    // Both notes are located at column 0.
    // A tempo change event is located at beat 2.
    // Result should be as described in the code below.
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    play->tempo = 60;
    Event_set_int(ev1_on, 0, 0);
    Event_set_int(ev1_on, 1, -1);
    Event_set_int(ev1_on, 2, NOTE_TABLE_MIDDLE_OCTAVE - 1);
    Event_set_int(ev1_on, 3, 1);
    Event_set_int(ev2_on, 0, 0);
    Event_set_int(ev2_on, 1, -1);
    Event_set_int(ev2_on, 2, NOTE_TABLE_MIDDLE_OCTAVE);
    Event_set_int(ev2_on, 3, 1);
    Event_set_pos(ev2_on, Reltime_set(RELTIME_AUTO, 0, RELTIME_BEAT / 4));
    col = Pattern_col(pat, 0);
    if (!Column_ins(col, ev2_on))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    ret = Pattern_mix(pat, 128, 0, play);
    fail_unless(ret == 72,
            "Pattern_mix() mixed %lu frames instead of 72.", (unsigned long)ret);
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i % 4 == 2)
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
    for (int i = 42; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    // Test again in small parts
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    play->tempo = 60;
    for (int i = 0; i < 256; ++i)
    {
        if (Pattern_mix(pat, i + 1, i, play) < 1)
        {
            fail_unless(i == 72,
                    "Pattern_mix() reached the end in %d samples instead of 72.", i);
            break;
        }
        fail_if(i == 72,
                "Pattern_mix() didn't reach the end in 72 samples.");
    }
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i % 4 == 2)
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
    for (int i = 42; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 4:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note #1 frequency is 1 Hz (1 cycle/beat).
    // Note #1 starts at the beginning and is released at frame 2
    // Note #2 frequency is 2 Hz (2 cycles/beat).
    // Note #2 starts at position 0:(1/4) (frame 2) and plays until the end
    // Both notes are located at column 0.
    // A tempo change event is located at beat 2.
    // Note #3 frequency is 1 Hz (0.5 cycles/beat).
    // Note #3 starts at position 3:0 (frame 20) and plays until the end of pattern
    // Note #3 is located at column 1.
    // Result should be as described in the code below.
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    play->tempo = 60;
    Event_set_int(ev3_on, 0, 0);
    Event_set_int(ev3_on, 1, -1);
    Event_set_int(ev3_on, 2, NOTE_TABLE_MIDDLE_OCTAVE - 1);
    Event_set_int(ev3_on, 3, 1);
    Event_set_pos(ev3_on, Reltime_set(RELTIME_AUTO, 3, 0));
    col = Pattern_col(pat, 1);
    if (!Column_ins(col, ev3_on))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    ret = Pattern_mix(pat, 128, 0, play);
    fail_unless(ret == 72,
            "Pattern_mix() mixed %lu frames instead of 72.", (unsigned long)ret);
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i >= 20)
        {
            if (i % 8 == 4 || i % 4 == 2)
            {
                fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                        "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
            }
            else
            {
                fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                        "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
            }
        }
        else if (i % 4 == 2)
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
    for (int i = 42; i < 72; ++i)
    {
        if (i % 8 == 4)
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
    for (int i = 72; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    // Test again in small parts
    for (int i = 0; i < 256; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        Channel_reset(play->channels[i]);
    }
    play->mode = PLAY_PATTERN;
    Voice_pool_reset(play->voice_pool);
    Reltime_init(&play->pos);
    play->tempo = 60;
    for (int i = 0; i < 256; ++i)
    {
        if (Pattern_mix(pat, i + 1, i, play) < 1)
        {
            fail_unless(i == 72,
                    "Pattern_mix() reached the end in %d samples instead of 72.", i);
            break;
        }
        fail_if(i == 72,
                "Pattern_mix() didn't reach the end in 72 samples.");
    }
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i >= 20)
        {
            if (i % 8 == 4 || i % 4 == 2)
            {
                fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                        "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
            }
            else
            {
                fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                        "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
            }
        }
        else if (i % 4 == 2)
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
    for (int i = 42; i < 72; ++i)
    {
        if (i % 8 == 4)
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
    for (int i = 72; i < 256; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }

    del_Note_table(notes);
    del_Instrument(ins);
    del_Pattern(pat);
}
END_TEST

#ifndef NDEBUG
START_TEST (mix_break_pat_null)
{
    Playdata* play = init_play();
    if (play == NULL) return;
    Pattern_mix(NULL, 1, 0, play);
}
END_TEST

START_TEST (mix_break_offset_inv)
{
    Playdata* play = init_play();
    if (play == NULL) return;
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        return;
    }
    Pattern_mix(pat, 0, 0, play);
    del_Pattern(pat);
}
END_TEST

START_TEST (mix_break_play_null)
{
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        fprintf(stderr, "new_Pattern() returned NULL -- out of memory?\n");
        return;
    }
    Pattern_mix(pat, 1, 0, NULL);
    del_Pattern(pat);
}
END_TEST
#endif


Suite* Pattern_suite(void)
{
    Suite* s = suite_create("Pattern");
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
    tcase_add_test_raise_signal(tc_mix, mix_break_pat_null, SIGABRT);
    tcase_add_test_raise_signal(tc_mix, mix_break_offset_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_mix, mix_break_play_null, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Pattern_suite();
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


