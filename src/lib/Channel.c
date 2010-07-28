

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
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Channel.h>
#include <Channel_state.h>

#include <kunquat/limits.h>
#include <Reltime.h>
#include <Event.h>
#include <Event_handler.h>
#include <Event_channel.h>
#include <Event_ins.h>
#include <Event_channel_note_off.h>
#include <Column.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


Channel* new_Channel(Ins_table* insts,
                     int num,
                     Voice_pool* pool,
                     double* tempo,
                     uint32_t* freq)
{
    assert(insts != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(pool != NULL);
    assert(tempo != NULL);
    assert(freq != NULL);
    Channel* ch = xalloc(Channel);
    if (ch == NULL)
    {
        return NULL;
    }
/*    ch->note_off = (Event*)new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
    if (ch->note_off == NULL)
    {
        xfree(ch);
        return NULL;
    } */
    ch->note_off = NULL;
/*    ch->single = (Event*)new_Event_voice_note_on(Reltime_set(RELTIME_AUTO, -1, 0));
    if (ch->single == NULL)
    {
        del_Event(ch->note_off);
        xfree(ch);
        return NULL;
    } */
    ch->single = NULL;
    if (!Channel_state_init(&ch->init_state, num, &ch->mute))
    {
        xfree(ch);
        return NULL;
    }
    ch->init_state.insts = insts;
    ch->init_state.fg_count = 0;
    ch->init_state.pool = pool;
    ch->init_state.tempo = tempo;
    ch->init_state.freq = freq;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ch->init_state.fg[i] = NULL;
        ch->init_state.fg_id[i] = 0;
    }
    Channel_state_copy(&ch->cur_state, &ch->init_state);
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
                        uint32_t freq,
                        Event_handler* eh)
{
    assert(ch != NULL);
    assert(pool != NULL);
    (void)pool; // FIXME: remove?
//  assert(citer != NULL);
    assert(start != NULL);
    assert(end != NULL);
    assert(offset <= nframes);
    assert(tempo > 0);
    assert(freq > 0);
    assert(eh != NULL);
    Event* next = ch->single;
    if (true || Reltime_cmp(Event_get_pos(next), Reltime_init(RELTIME_AUTO)) < 0) // FIXME: true
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
        LFO_set_mix_rate(&ch->cur_state.vibrato, freq);
        LFO_set_tempo(&ch->cur_state.vibrato, tempo);
        LFO_set_mix_rate(&ch->cur_state.tremolo, freq);
        LFO_set_tempo(&ch->cur_state.tremolo, tempo);
        Slider_set_mix_rate(&ch->cur_state.panning_slider, freq);
        Slider_set_tempo(&ch->cur_state.panning_slider, tempo);
        LFO_set_mix_rate(&ch->cur_state.autowah, freq);
        LFO_set_tempo(&ch->cur_state.autowah, tempo);
        uint32_t to_be_mixed = MIN(abs_pos, nframes);
        for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
        {
            if (ch->cur_state.fg[i] != NULL)
            {
                ch->cur_state.fg[i] = Voice_pool_get_voice(pool,
                                              ch->cur_state.fg[i],
                                              ch->cur_state.fg_id[i]);
                if (ch->cur_state.fg[i] != NULL)
                {
//                    fprintf(stderr, "checking priority %p\n", (void*)&ch->cur_state.fg[i]->prio);
                    assert(ch->cur_state.fg[i]->prio > VOICE_PRIO_INACTIVE);
                    Voice_mix(ch->cur_state.fg[i], to_be_mixed, mixed, freq, tempo);
                }
            }
        }
        if (Slider_in_progress(&ch->cur_state.panning_slider) &&
                to_be_mixed > mixed)
        {
            Slider_skip(&ch->cur_state.panning_slider, to_be_mixed - mixed);
        }
        mixed = to_be_mixed;
        if (Reltime_cmp(next_pos, end) >= 0)
        {
            break;
        }
        assert(next != NULL);
        if (EVENT_IS_CHANNEL(Event_get_type(next)) ||
                EVENT_IS_INS(Event_get_type(next)) ||
                EVENT_IS_GENERATOR(Event_get_type(next)) ||
                EVENT_IS_DSP(Event_get_type(next)))
        {
            Event_handler_handle(eh, ch->init_state.num,
                                 Event_get_type(next),
                                 Event_get_fields(next));
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
            continue;
        }
        Reltime_copy(next_pos, Event_get_pos(next));
    }
    return;
}


void Channel_update_state(Channel* ch, uint32_t mixed)
{
    assert(ch != NULL);
    (void)ch;
    (void)mixed;
    return;
}


void Channel_reset(Channel* ch)
{
    assert(ch != NULL);
    Channel_state_copy(&ch->cur_state, &ch->init_state);
//    Channel_state_copy(&ch->new_state, &ch->init_state);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ch->cur_state.fg[i] = NULL;
        ch->cur_state.fg_id[i] = 0;
    }
    ch->cur_state.fg_count = 0;
    return;
}


void del_Channel(Channel* ch)
{
    if (ch == NULL)
    {
        return;
    }
//    assert(ch->note_off != NULL);
//    assert(ch->single != NULL);
//    del_Event(ch->note_off);
//    del_Event(ch->single);
    Channel_state_uninit(&ch->init_state);
    // cur_state must not be uninitialised -- it merely contains references
    // to dynamic structures in init_state.
    xfree(ch);
    return;
}


