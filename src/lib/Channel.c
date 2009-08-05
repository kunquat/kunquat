

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
#include <stdbool.h>
#include <stdio.h>

#include <Channel.h>
#include <Channel_state.h>

#include <kunquat/limits.h>
#include <Reltime.h>
#include <Event.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <Column.h>

#include <xmemory.h>


Channel* new_Channel(Ins_table* insts, int num)
{
    assert(insts != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    Channel* ch = xalloc(Channel);
    if (ch == NULL)
    {
        return NULL;
    }
    ch->note_off = (Event*)new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
    if (ch->note_off == NULL)
    {
        xfree(ch);
        return NULL;
    }
    ch->single = (Event*)new_Event_voice_note_on(Reltime_set(RELTIME_AUTO, -1, 0));
    if (ch->single == NULL)
    {
        del_Event(ch->note_off);
        xfree(ch);
        return NULL;
    }
    Channel_state_init(&ch->init_state, num, &ch->mute);
    Channel_state_copy(&ch->cur_state, &ch->init_state);
    Channel_state_copy(&ch->new_state, &ch->init_state);
    ch->insts = insts;
    ch->fg_count = 0;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ch->fg[i] = NULL;
        ch->fg_id[i] = 0;
    }
    return ch;
}


void Channel_set_voices(Channel* ch,
                        Voice_pool* pool,
                        Column_iter* citer,
                        Reltime* start,
                        Reltime* end,
                        uint32_t offset,
                        double tempo,
                        uint32_t freq)
{
    assert(ch != NULL);
    assert(pool != NULL);
//  assert(citer != NULL);
    assert(start != NULL);
    assert(end != NULL);
    assert(tempo > 0);
    assert(freq > 0);
    Event* next = ch->single;
    if (Reltime_cmp(Event_get_pos(ch->single), Reltime_init(RELTIME_AUTO)) < 0)
    {
        next = NULL;
        if (citer != NULL)
        {
            next = Column_iter_get(citer, start);
        }
        if (next == NULL)
        {
            return;
        }
    }
    else
    {
        Event_set_pos(ch->single, start);
    }
    Reltime* next_pos = Event_get_pos(next);
    while (Reltime_cmp(next_pos, end) < 0)
    {
        assert(Reltime_cmp(start, next_pos) <= 0);
        if (Event_get_type(next) == EVENT_VOICE_NOTE_ON)
        {
            for (int i = 0; i < ch->fg_count; ++i)
            {
                if (ch->fg[i] != NULL)
                {
                    // move the old Voice to the background
                    ch->fg[i] = Voice_pool_get_voice(pool, ch->fg[i], ch->fg_id[i]);
                    if (ch->fg[i] == NULL)
                    {
                        // The Voice has been given to another channel -- giving up
                        continue;
                    }
                    Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
                    uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq)
                            + offset;
                    // FIXME: it seems that we may use ch->note_off for several Voices
                    //        -- this leads to slightly incorrect Note Off positions
                    if (!Voice_add_event(ch->fg[i], ch->note_off, abs_pos))
                    {
                        // Kill the Voice so that it doesn't
                        // stay active indefinitely
                        Voice_reset(ch->fg[i]);
                        ch->fg[i] = NULL;
                        ch->fg_id[i] = 0;
                        // TODO: notify in case of failure
                    }
                    ch->fg[i] = NULL;
                }
            }
            ch->fg_count = 0;
            int64_t* num = Event_get_field(next, 3);
            assert(num != NULL);
            if (*num <= 0)
            {
                next = NULL;
                if (citer != NULL)
                {
                    next = Column_iter_get_next(citer);
                }
                if (next == NULL)
                {
                    break;
                }
                next_pos = Event_get_pos(next);
                continue;
            }
            Instrument* ins = Ins_table_get(ch->insts, *num);
            if (ins == NULL)
            {
                next = NULL;
                if (citer != NULL)
                {
                    next = Column_iter_get_next(citer);
                }
                if (next == NULL)
                {
                    break;
                }
                next_pos = Event_get_pos(next);
                continue;
            }
            // allocate new Voices
            ch->fg_count = Instrument_get_gen_count(ins);
            ch->new_state.panning_slide = 0;
            for (int i = 0; i < ch->fg_count; ++i)
            {
                assert(Instrument_get_gen(ins, i) != NULL);
                ch->fg[i] = Voice_pool_get_voice(pool, NULL, 0);
                assert(ch->fg[i] != NULL);
                ch->fg_id[i] = Voice_id(ch->fg[i]);
                Voice_init(ch->fg[i],
                           Instrument_get_gen(ins, i),
                           &ch->cur_state,
                           &ch->new_state,
                           freq,
                           tempo);
                Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
                uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq)
                        + offset;
                if (!Voice_add_event(ch->fg[i], next, abs_pos))
                {
                    // This really shouldn't occur here!
                    //  - implies that the Voice is uninitialised
                    //    or cannot contain any events
                    assert(false);
                }
            }
        }
        else if (ch->fg_count > 0 &&
                !EVENT_IS_GLOBAL(Event_get_type(next)))
        {
            bool voices_active = false;
            for (int i = 0; i < ch->fg_count; ++i)
            {
                if (ch->fg[i] != NULL)
                {
                    ch->fg[i] = Voice_pool_get_voice(pool, ch->fg[i], ch->fg_id[i]);
                    if (ch->fg[i] == NULL)
                    {
                        // The Voice has been given to another channel -- giving up
                        continue;
                    }
                    voices_active = true;
                    Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
                    uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq) + offset;
                    if (!Voice_add_event(ch->fg[i], next, abs_pos))
                    {
                        Voice_reset(ch->fg[i]);
                        ch->fg[i] = NULL;
                        ch->fg_id[i] = 0;
                        // TODO: notify in case of failure
                    }
                }
            }
            if (!voices_active)
            {
                ch->fg_count = 0;
                // TODO: Insert Channel effect processing here
            }
        }
        if (next == ch->single)
        {
            Event_set_pos(ch->single, Reltime_set(RELTIME_AUTO, -1, 0));
            next = NULL;
            if (citer != NULL)
            {
                next = Column_iter_get(citer, start);
            }
        }
        else
        {
            next = NULL;
            if (citer != NULL)
            {
                next = Column_iter_get_next(citer);
            }
        }
        if (next == NULL)
        {
            break;
        }
        next_pos = Event_get_pos(next);
    }
    return;
}


void Channel_update_state(Channel* ch, uint32_t mixed)
{
    assert(ch != NULL);
    if (ch->new_state.panning_slide != 0 && ch->new_state.panning_slide_prog < mixed)
    {
        uint32_t frames_left = mixed - ch->new_state.panning_slide_prog;
        ch->new_state.panning += ch->new_state.panning_slide_update * frames_left;
        ch->new_state.panning_slide_frames -= frames_left;
        if (ch->new_state.panning_slide_frames <= 0)
        {
            if (ch->new_state.num == 1)
            {
                fprintf(stderr, "slide_frames: %f \n", ch->new_state.panning_slide_frames);
                fprintf(stderr, "slide_target: %f \n", ch->new_state.panning_slide_target);
            }
            ch->new_state.panning = ch->new_state.panning_slide_target;
            ch->new_state.panning_slide = 0;
        }
        else if (ch->new_state.panning_slide == 1)
        {
            if (ch->new_state.panning > ch->new_state.panning_slide_target)
            {
                ch->new_state.panning = ch->new_state.panning_slide_target;
                ch->new_state.panning_slide = 0;
            }
        }
        else
        {
            assert(ch->new_state.panning_slide == -1);
            if (ch->new_state.panning < ch->new_state.panning_slide_target)
            {
                ch->new_state.panning = ch->new_state.panning_slide_target;
                ch->new_state.panning_slide = 0;
            }
        }
    }
    Channel_state_copy(&ch->cur_state, &ch->new_state);
    ch->cur_state.panning_slide_prog = ch->new_state.panning_slide_prog = 0;
    return;
}


void Channel_reset(Channel* ch)
{
    assert(ch != NULL);
    for (int i = 0; i < ch->fg_count; ++i)
    {
        ch->fg[i] = NULL;
        ch->fg_id[i] = 0;
    }
    ch->fg_count = 0;
    return;
}


void del_Channel(Channel* ch)
{
    assert(ch != NULL);
    assert(ch->note_off != NULL);
    assert(ch->single != NULL);
    del_Event(ch->note_off);
    del_Event(ch->single);
    xfree(ch);
    return;
}


