

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Channel.h>
#include <Channel_state.h>

#include <kunquat/limits.h>
#include <Reltime.h>
#include <Event.h>
#include <Event_channel.h>
#include <Event_ins.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <Column.h>
#include <math_common.h>

#include <xmemory.h>


Channel* new_Channel(Ins_table* insts, int num, Event_queue* ins_events)
{
    assert(insts != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(ins_events != NULL);
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
    ch->cur_inst = 0;
    ch->insts = insts;
    ch->ins_events = ins_events;
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
                        uint32_t nframes,
                        uint32_t offset,
                        double tempo,
                        uint32_t freq)
{
    assert(ch != NULL);
    assert(pool != NULL);
//  assert(citer != NULL);
    assert(start != NULL);
    assert(end != NULL);
    assert(offset < nframes);
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
    }
    else
    {
        Event_set_pos(ch->single, start);
    }
    Reltime* next_pos = Reltime_set(RELTIME_AUTO, INT64_MAX, KQT_RELTIME_BEAT - 1);
    if (next != NULL)
    {
        Reltime_copy(next_pos, Event_get_pos(next));
    }
    assert(!(Reltime_get_beats(next_pos) == INT64_MAX) || (next == NULL));
    assert(!(next == NULL) || (Reltime_get_beats(next_pos) == INT64_MAX));
    uint32_t mixed = offset;
//    fprintf(stderr, "Entering Channel mixing, mixed: %" PRIu32 ", nframes: %" PRIu32 "\n",
//            mixed, nframes);
    while (Reltime_cmp(next_pos, end) < 0 || mixed < nframes)
    {
        assert(Reltime_cmp(start, next_pos) <= 0);
        assert(!(Reltime_get_beats(next_pos) == INT64_MAX) || (next == NULL));
        assert(!(next == NULL) || (Reltime_get_beats(next_pos) == INT64_MAX));
        Reltime* rel_offset = Reltime_sub(RELTIME_AUTO, next_pos, start);
        uint32_t abs_pos = Reltime_toframes(rel_offset, tempo, freq);
//        fprintf(stderr, "mixed: %u, abs_pos: %u, nframes: %u\n",
//                (unsigned int)mixed, (unsigned int)abs_pos, (unsigned int)nframes);
        if (abs_pos < UINT32_MAX - offset)
        {
            abs_pos += offset;
        }
        uint32_t to_be_mixed = MIN(abs_pos, nframes);
        for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
        {
            if (ch->fg[i] != NULL)
            {
#if 0
                if (to_be_mixed == mixed)
                {
                    fprintf(stderr, "to_be_mixed = mixed = %" PRIu32, mixed);
                    fprintf(stderr, ", next event is %p\n", (void*)next);
                }
#endif
                Voice_mix(ch->fg[i], to_be_mixed, mixed, freq, tempo);
            }
        }
        mixed = to_be_mixed;
        if (Reltime_cmp(next_pos, end) >= 0)
        {
#if 0
            if (next && Event_get_type(next) == EVENT_VOICE_NOTE_ON)
            {
                fprintf(stderr, "missed note on event %p at ", (void*)next);
                fprintf(stderr, "[%" PRId64 ", %" PRId32 "]", next_pos->beats, next_pos->rem);
                fprintf(stderr, ", end is ");
                fprintf(stderr, "[%" PRId64 ", %" PRId32 "]", end->beats, end->rem);
                fprintf(stderr, "\n");
            }
#endif
            break;
        }
        assert(next != NULL);
        if (Event_get_type(next) == EVENT_VOICE_NOTE_OFF ||
                Event_get_type(next) == EVENT_VOICE_NOTE_ON)
        {
            for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
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
                    // FIXME: it seems that we may use ch->note_off for several Voices
                    //        -- this leads to slightly incorrect Note Off positions
                    Event_voice_process((Event_voice*)ch->note_off, ch->fg[i]);
                    ch->fg[i]->prio = VOICE_PRIO_BG;
                    Voice_pool_fix_priority(pool, ch->fg[i]);
//                    ch->fg[i]->prio = VOICE_PRIO_INACTIVE;
#if 0
                    if (!Voice_add_event(ch->fg[i], ch->note_off, abs_pos))
                    {
                        // Kill the Voice so that it doesn't
                        // stay active indefinitely
                        Voice_reset(ch->fg[i]);
                        ch->fg[i] = NULL;
                        ch->fg_id[i] = 0;
                        // TODO: notify in case of failure
                    }
#endif
                    ch->fg[i] = NULL;
                }
            }
        }
        if (Event_get_type(next) == EVENT_VOICE_NOTE_ON)
        {
//            fprintf(stderr, "handling note on event %p\n", (void*)next);
            ch->fg_count = 0;
            if (ch->cur_inst == 0)
            {
                next = NULL;
                if (citer != NULL)
                {
                    next = Column_iter_get_next(citer);
                }
                if (next == NULL)
                {
                    Reltime_set(next_pos, INT64_MAX, KQT_RELTIME_BEAT - 1);
//                    fprintf(stderr, "continue %d\n", __LINE__);
                    continue;
                }
                Reltime_copy(next_pos, Event_get_pos(next));
//                fprintf(stderr, "continue %d\n", __LINE__);
                continue;
            }
            Instrument* ins = Ins_table_get(ch->insts, ch->cur_inst);
            if (ins == NULL)
            {
                next = NULL;
                if (citer != NULL)
                {
                    next = Column_iter_get_next(citer);
                }
                if (next == NULL)
                {
                    Reltime_set(next_pos, INT64_MAX, KQT_RELTIME_BEAT - 1);
//                    fprintf(stderr, "continue %d\n", __LINE__);
                    continue;
                }
                Reltime_copy(next_pos, Event_get_pos(next));
//                fprintf(stderr, "continue %d\n", __LINE__);
                continue;
            }
            // allocate new Voices
            ch->fg_count = 0;
            ch->new_state.panning_slide = 0;
            for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
            {
                if (Instrument_get_gen(ins, i) == NULL)
                {
                    continue;
                }
                ++ch->fg_count;
                ch->fg[i] = Voice_pool_get_voice(pool, NULL, 0);
                assert(ch->fg[i] != NULL);
//                fprintf(stderr, "allocated Voice %p\n", (void*)ch->fg[i]);
                ch->fg_id[i] = Voice_id(ch->fg[i]);
                Voice_init(ch->fg[i],
                           Instrument_get_gen(ins, i),
                           &ch->cur_state,
                           &ch->new_state,
                           freq,
                           tempo);
                Event_voice_process((Event_voice*)next, ch->fg[i]);
                Voice_pool_fix_priority(pool, ch->fg[i]);
#if 0
                if (!Voice_add_event(ch->fg[i], next, abs_pos))
                {
                    // This really shouldn't occur here!
                    //  - implies that the Voice is uninitialised
                    //    or cannot contain any events
                    assert(false);
                }
#endif
            }
        }
        else if (ch->fg_count > 0 &&
                 Event_get_type(next) != EVENT_VOICE_NOTE_OFF &&
                 (EVENT_IS_GENERAL(Event_get_type(next)) ||
                  EVENT_IS_VOICE(Event_get_type(next))))
        {
            bool voices_active = false;
            ch->fg_count = 0;
            for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
            {
                if (ch->fg[i] != NULL)
                {
                    ch->fg[i] = Voice_pool_get_voice(pool, ch->fg[i], ch->fg_id[i]);
                    if (ch->fg[i] == NULL)
                    {
                        // The Voice has been given to another channel -- giving up
                        continue;
                    }
                    ++ch->fg_count;
                    voices_active = true;
                    Event_voice_process((Event_voice*)next, ch->fg[i]);
#if 0
                    if (!Voice_add_event(ch->fg[i], next, abs_pos))
                    {
                        Voice_reset(ch->fg[i]);
                        ch->fg[i] = NULL;
                        ch->fg_id[i] = 0;
                        // TODO: notify in case of failure
                    }
#endif
                }
            }
            if (!voices_active)
            {
                ch->fg_count = 0;
                // TODO: Insert Channel effect processing here
            }
        }
        else if (EVENT_IS_INS(Event_get_type(next)))
        {
            Instrument* ins = Ins_table_get(ch->insts, ch->cur_inst);
            if (ins != NULL)
            {
                Event_ins_set_params((Event_ins*)next, Instrument_get_params(ins));
                Event_queue_ins(ch->ins_events, next, abs_pos);
            }
        }
        else if (EVENT_IS_CHANNEL(Event_get_type(next)))
        {
            Event_channel_process((Event_channel*)next, ch);
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
            Reltime_set(next_pos, INT64_MAX, KQT_RELTIME_BEAT - 1);
//            fprintf(stderr, "continue %d\n", __LINE__);
            continue;
        }
        Reltime_copy(next_pos, Event_get_pos(next));
//        fprintf(stderr, "continue %d\n", __LINE__);
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
    Channel_state_copy(&ch->cur_state, &ch->init_state);
    Channel_state_copy(&ch->new_state, &ch->init_state);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
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


